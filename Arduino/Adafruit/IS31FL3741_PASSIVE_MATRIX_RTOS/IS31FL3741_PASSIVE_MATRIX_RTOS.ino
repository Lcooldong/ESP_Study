#include <Adafruit_IS31FL3741.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <NimBLEDevice.h>
#include <SPI.h>

#include "Button.h"
#include "Animation.h"

#define DEBUG_SERIAL Serial

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define TFT_RST           1  
#define TFT_CS            2  
#define SWITCH_PIN        3
#define TFT_DC            4
#define I2C_SDA_PIN       5
#define I2C_SCL_PIN       6
#define MATRIX_SHTDN      8

#define UART_TX           43
#define UART_RX           44

const int MATRIX_WIDTH    = 8;   
const int MATRIX_HEIGHT   = 8;  
const int HEADER_CURSOR_X = 20;
const int HEADER_CURSOR_Y = 20; 

Adafruit_IS31FL3741 ledMatrix;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCharacteristic = nullptr;

// 전역 자원 (두 코어가 공유하는 데이터)
uint8_t ledBuffer[MATRIX_WIDTH * MATRIX_HEIGHT * 3] = {0,};
uint8_t lastTftBuffer[MATRIX_WIDTH * MATRIX_HEIGHT * 3] = {255,}; // 화면 부분 갱신(Delta)용 캐시

uint8_t tftR = 0;
uint8_t tftG = 255;
uint8_t tftB = 0;

volatile bool currentBleConnected = false;  // volatile 처리로 멀티코어 동기화 보장
bool lastBleConnected = false;     

Button myBtn(SWITCH_PIN, true, 50);
bool longPressExecuted = false;
int currentAnimationMode = 0;

// FreeRTOS 타스크 핸들 선언
TaskHandle_t hTask_BLE = NULL;
TaskHandle_t hTask_DisplayHardware = NULL;

// 함수 선언부
void Task_BLE_Code(void * pvParameters);
void Task_DisplayHardware_Code(void * pvParameters);
void initBLE();
void updateTftDisplay();
void drawMonoPixel(int16_t x, int16_t y, uint8_t brightness);
void buttonInit(uint8_t button_pin);
void buttonTask();
bool i2cCheckSingleShot();
void breath(int interval);
void initTFTDisplay(const char* address);

// BLE 서버 연결 상태 콜백
class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
      DEBUG_SERIAL.println("PC/스마트폰이 연결되었습니다! (Core 0)");
      currentBleConnected = true;
  }
  
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
      DEBUG_SERIAL.println("연결 해제됨. 다시 광고 시작...");
      currentBleConnected = false; 
      NimBLEDevice::startAdvertising(); 
  }
};

// BLE 데이터 수신 콜백
class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        std::string value = pChar->getValue();
        
        // [수정] 64에서 192바이트로 변경!
        if (value.length() == 192) {
            memcpy(ledBuffer, value.data(), 192);
        } 
    }
};

void setup() {
  DEBUG_SERIAL.begin(115200);
  NimBLEDevice::init("ESP32_NimBLE_MATRIX");

  // 1. TFT 기본 부팅 레이아웃 초기화
  memset(lastTftBuffer, 255, sizeof(lastTftBuffer));
  std::string myMacAddress = NimBLEDevice::getAddress().toString();
  
  initTFTDisplay(myMacAddress.c_str());


  // 2. 물리 핀 및 입력부 세팅
  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, LOW);  
  buttonInit(SWITCH_PIN);

  // 3. I2C 하드웨어 버스 점검
  Serial.println("I2C Bus Init...");
  if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
    Serial.println("I2C Init Failed");
  } else {
    i2cCheckSingleShot();
  }

  // 4. IS31FL3741 드라이버 칩 활성화
  for(int i=0; i<10; i++){
    if (!ledMatrix.begin(IS3741_ADDR_DEFAULT, &Wire)) {
      DEBUG_SERIAL.print("["); DEBUG_SERIAL.print(i + 1); DEBUG_SERIAL.println("] IS31FL3741 탐색 실패. 재시도...");
      delay(100);
    } else {
      Serial.println("IS31FL3741 초기화 성공!");
      Wire.setClock(800000); // 고속 드라이빙 세팅
      ledMatrix.setLEDscaling(0xFF);
      ledMatrix.setGlobalCurrent(0xFF);
      ledMatrix.enable(true); 
      ledMatrix.fill(0);
      break;
    }
  }

  // -------------------------------------------------------------------------
  // 5. FreeRTOS 멀티코어 타스크 생성
  // -------------------------------------------------------------------------
  
  // [Core 0] BLE 전용 스레드 가동
  xTaskCreatePinnedToCore(
    Task_BLE_Code,            // 실행할 함수
    "Task_BLE",               // 타스크 명칭
    4096,                     // 스택 크기
    NULL,                     // 파라미터
    3,                        // 우선순위 (높음)
    &hTask_BLE,               // 핸들러 주소
    0                         // ★ Core 0 할당
  );

  // [Core 1] 디스플레이 및 물리 제어 하드웨어 스레드 가동
  xTaskCreatePinnedToCore(
    Task_DisplayHardware_Code,// 실행할 함수
    "Task_DisplayHardware",   // 타스크 명칭
    4096,                     // 스택 크기
    NULL,                     // 파라미터
    1,                        // 우선순위 (기본)
    &hTask_DisplayHardware,   // 핸들러 주소
    1                         // ★ Core 1 할당
  );

  DEBUG_SERIAL.println("FreeRTOS 엔진 제어권 분할 성공. 메인 루프를 종료합니다.");
}

// 아두이노 기본 loop()는 쓰지 않습니다. 
// FreeRTOS 타스크들이 완벽히 백그라운드 코어를 선점했으므로 메인 루프 인스턴스는 자원을 반납합니다.
void loop() {
  vTaskDelete(NULL); 
}

// =========================================================================
// [Core 0 번에서 무한 루프 구동될 통신 타스크]
// =========================================================================
void Task_BLE_Code(void * pvParameters) {
  initBLE(); // BLE 스택 초기화 및 광고 시작
  
  for(;;) {
    // NimBLE은 하드웨어 내부 이벤트를 비동기로 잡기 때문에, 
    // 본 루프에서는 CPU 스케줄러가 터지지 않도록 안정성용 최소 딜레이(10ms)만 부여합니다.
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

// =========================================================================
// [Core 1 번에서 무한 루프 구동될 하드웨어 제어/UI 타스크]
// =========================================================================
void Task_DisplayHardware_Code(void * pvParameters) {
  unsigned long tftUpdateMillis = 0;
  unsigned long matrixUpdateMillis = 0;
  
  for(;;) {
    unsigned long curMillis = millis();
    
    // 1. 버튼 스캔 인터럽트 태스크
    buttonTask();
    if (!currentBleConnected) {
      if (currentAnimationMode > 0) {
        updateAnimation(currentAnimationMode);
      } else {
        memset(ledBuffer, 0, 192); // 모드 0이면 끄기
      }
    }

    // 2. 내장 LED 숨쉬기 효과 (Non-Blocking)
    breath(5);

    // 3. I2C 8x8 매트릭스 LED 리프레시 (30ms 주기, 약 33 FPS 제한)
    if (curMillis - matrixUpdateMillis >= 30) {
      matrixUpdateMillis = curMillis;
      for (int i = 0; i < 64; i++) {
        int y = i / 8;
        int x = i % 8;
        
        // 192바이트 배열에서 해당 픽셀의 파이썬 수신 RGB 값 추출
        uint8_t r = ledBuffer[i*3];
        uint8_t g = ledBuffer[i*3+1];
        uint8_t b = ledBuffer[i*3+2];
        
        // ★ [핵심] 수신된 RGB 값 중 가장 강한(가장 밝은) 값을 물리 LED의 단색 밝기로 취합니다.
        uint8_t max_val = r;
        if (g > max_val) max_val = g;
        if (b > max_val) max_val = b;
        
        drawMonoPixel(x, y, max_val);
      }
    }
    
    // 4. SPI TFT LCD 모니터링 창 갱신 (20ms 주기, 부드러운 드로잉 제어)
    if (curMillis - tftUpdateMillis >= 20) {
      tftUpdateMillis = curMillis;
      updateTftDisplay();
    }

    // 5. BLE 연결선 끊김/생성 UI 변경 스위칭 로직
    if (currentBleConnected != lastBleConnected) {
      lastBleConnected = currentBleConnected; 

      tft.fillRect(20, HEADER_CURSOR_Y + 40, 200, 15, ST77XX_BLACK); 
      tft.setCursor(20, HEADER_CURSOR_Y + 40);
      tft.setTextSize(1);

      if (currentBleConnected) {
        tft.setTextColor(ST77XX_BLUE);
        tft.print("BLE: CONNECTED");
      } else {
        tft.setTextColor(ST77XX_RED);
        tft.print("BLE: DISCONNECTED");
      }
    }

    // ★ 매우 중요: RTOS 타스크 내부의 무한루프는 CPU 독점을 방지하기 위해 
    // 최소 2ms의 강제 휴식 타이밍(Yielding)을 반드시 포함해야 Watchdog 리셋이 안 걸립니다.
    vTaskDelay(pdMS_TO_TICKS(2)); 
  }
}

// --- 하위 유틸리티 함수 스펙 유지 ---

void initBLE(){
  
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::WRITE | 
                      NIMBLE_PROPERTY::WRITE_NR
                    );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->enableScanResponse(true);
  
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); // 수신 감도 세팅 극대화
  NimBLEDevice::startAdvertising();

  DEBUG_SERIAL.println("NimBLE Core 0 서버 매핑 완료.");
}

void updateTftDisplay() {
  int startX = 40;  
  int startY = HEADER_CURSOR_Y + 60; 
  int blockSize = 18; 
  int gap = 2;        

  // 192바이트를 순회하며 개별 픽셀을 풀 컬러로 렌더링
  for(int i=0; i<64; i++){
    uint8_t r = ledBuffer[i*3];
    uint8_t g = ledBuffer[i*3+1];
    uint8_t b = ledBuffer[i*3+2];
    
    // 이전 화면과 RGB 3가지 중 하나라도 다르면 그 픽셀을 갱신합니다.
    if(r != lastTftBuffer[i*3] || g != lastTftBuffer[i*3+1] || b != lastTftBuffer[i*3+2]) {
      lastTftBuffer[i*3] = r;
      lastTftBuffer[i*3+1] = g;
      lastTftBuffer[i*3+2] = b;
      
      uint16_t blockColor = tft.color565(r, g, b);
      
      // 완전히 꺼졌을 때는 기본 테마인 짙은 회색으로 표현
      if(r == 0 && g == 0 && b == 0) {
        blockColor = tft.color565(40, 40, 40);
      }
      
      int y = i / 8;
      int x = i % 8;
      tft.fillRect(startX + (x * (blockSize + gap)), 
                   startY + (y * (blockSize + gap)), 
                   blockSize, blockSize, blockColor);
    }
  }
}

void drawMonoPixel(int16_t x, int16_t y, uint8_t brightness) {
  if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
    uint16_t lednum = (y * MATRIX_WIDTH) + x;
    if (lednum < 351) {
      ledMatrix.setLEDPWM(lednum, brightness); 
    }
  }
}

void buttonInit(uint8_t button_pin){
  pinMode(button_pin, INPUT_PULLUP);
  for(int i = 0; i < 5; i++) {
      myBtn.read();
      delay(5);
  }
  longPressExecuted = false;
}

void buttonTask(){
  myBtn.read();
  if (myBtn.pressedFor(2000)) {
      if (!longPressExecuted) {
          DEBUG_SERIAL.println("--- 2 Seconds Reached! ---"); // 2초간 눌렀을 때
          longPressExecuted = true;
          ESP.restart();
      }
  }
  if (myBtn.wasReleased()) {
      if (longPressExecuted) {
          longPressExecuted = false; 
          DEBUG_SERIAL.println("Button Released after Long Press"); // 2초 동안 누르고 땠을 때
          // digitalWrite(LED_BUILTIN, LOW);
      } else {
          DEBUG_SERIAL.println("--- Short Press Detected");         // 짧게 눌렀을 때
          currentAnimationMode++;
          if (currentAnimationMode > 3) currentAnimationMode = 0; // 3번 모드 다음엔 0(꺼짐)으로
          DEBUG_SERIAL.printf("Animation Mode Changed to: %d\n", currentAnimationMode);
          // digitalWrite(LED_BUILTIN, HIGH);
      }
  }
}

bool i2cCheckSingleShot() {
  int devicesFound = 0;
  Serial.println("\n--- I2C 스캔 시작 (setup) ---");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Found: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devicesFound++;
    }
    delay(1); 
  }
  Serial.print("Scan Done. Total Found: ");
  Serial.println(devicesFound);
  Serial.println("-----------------------------\n");
  return (devicesFound > 0);
}

void breath(int interval) {
  static int breathBrightness = 0; 
  static int breathAmount = 1;     
  static unsigned long lastBreathTime = 0; // [버그 수정] 로컬 static화하여 상태 보존

  unsigned long currentMillis = millis();
  if (currentMillis - lastBreathTime >= (unsigned long)interval) {
    lastBreathTime = currentMillis; 
    analogWrite(LED_BUILTIN, breathBrightness);
    breathBrightness += breathAmount;
    if (breathBrightness >= 255) {
      breathBrightness = 255;
      breathAmount = -1; 
    }
    else if (breathBrightness <= 0) {
      breathBrightness = 0;
      breathAmount = 1;  
    }
  }
}

void initTFTDisplay(const char* address){
  tft.init(240, 280);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(20, HEADER_CURSOR_Y);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Start LED Matrix");

  tft.setCursor(20, HEADER_CURSOR_Y + 25);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("MAC: ");
  tft.print(address);

  tft.setCursor(20, HEADER_CURSOR_Y + 40);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_RED);
  tft.print("BLE: DISCONNECTED");

  tft.drawRect(38, HEADER_CURSOR_Y + 58, 164, 164, ST77XX_WHITE);
}