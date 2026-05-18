#include <Adafruit_IS31FL3741.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <NimBLEDevice.h>
#include <SPI.h>

#include "Button.h"

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

const int MATRIX_WIDTH = 8;   
const int MATRIX_HEIGHT = 8;  

Adafruit_IS31FL3741 ledMatrix;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCharacteristic = nullptr;

// 전역 자원 (두 코어가 공유하는 데이터)
uint8_t ledBuffer[MATRIX_WIDTH * MATRIX_HEIGHT] = {0,};
uint8_t lastTftBuffer[MATRIX_WIDTH * MATRIX_HEIGHT] = {255,}; // 화면 부분 갱신(Delta)용 캐시

uint8_t tftR = 0;
uint8_t tftG = 255;
uint8_t tftB = 0;

volatile bool currentBleConnected = false;  // volatile 처리로 멀티코어 동기화 보장
bool lastBleConnected = false;     

Button myBtn(SWITCH_PIN, true, 50);
bool longPressExecuted = false;

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
        
        if (value.length() == 64) {
            // 스트리밍 데이터 (64바이트)
            memcpy(ledBuffer, value.data(), 64);
        } 
        else if (value.length() == 3) {
            // [추가] 색상 변경 명령어 (3바이트: R, G, B)
            tftR = value[0];
            tftG = value[1];
            tftB = value[2];
            
            // 색상이 완전히 바뀌었으므로, 다음 루프 때 모든 격자를 
            // 강제로 다시 그리도록 캐시 버퍼를 초기화해 줍니다.
            memset(lastTftBuffer, 255, sizeof(lastTftBuffer));
            
            DEBUG_SERIAL.printf("TFT 색상 변경됨 -> R:%d, G:%d, B:%d\n", tftR, tftG, tftB);
        }
    }
};

void setup() {
  DEBUG_SERIAL.begin(115200);

  // 1. TFT 기본 부팅 레이아웃 초기화
  tft.init(240, 280);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(20, 20);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Start LED Matrix");

  tft.setCursor(20, 40);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_RED);
  tft.print("BLE: DISCONNECTED");

  tft.drawRect(38, 68, 164, 164, ST77XX_WHITE);

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
    
    // 2. 내장 LED 숨쉬기 효과 (Non-Blocking)
    breath(5);

    // 3. I2C 8x8 매트릭스 LED 리프레시 (30ms 주기, 약 33 FPS 제한)
    if (curMillis - matrixUpdateMillis >= 30) {
      matrixUpdateMillis = curMillis;
      for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
          uint8_t brightness = ledBuffer[(y * MATRIX_WIDTH) + x];
          drawMonoPixel(x, y, brightness);
        }
      }
    }
    
    // 4. SPI TFT LCD 모니터링 창 갱신 (50ms 주기, 부드러운 드로잉 제어)
    if (curMillis - tftUpdateMillis >= 30) {
      tftUpdateMillis = curMillis;
      updateTftDisplay();
    }

    // 5. BLE 연결선 끊김/생성 UI 변경 스위칭 로직
    if (currentBleConnected != lastBleConnected) {
      lastBleConnected = currentBleConnected; 

      tft.fillRect(20, 40, 200, 20, ST77XX_BLACK); 
      tft.setCursor(20, 40);
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
  NimBLEDevice::init("ESP32_NimBLE_MATRIX");
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
  int startY = 70; 
  int blockSize = 18; 
  int gap = 2;        

  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      int index = (y * MATRIX_WIDTH) + x;
      uint8_t currentBrightness = ledBuffer[index];

      // 이전 화면과 밝기가 다를 때(또는 색상 변경으로 캐시가 초기화됐을 때)만 그립니다.
      if (currentBrightness != lastTftBuffer[index]) {
        lastTftBuffer[index] = currentBrightness; // 캐시 갱신
        
        // [수정] 수신된 흑백 밝기(0~255)를 기준 색상에 곱해서 농도 조절
        uint8_t r = (tftR * currentBrightness) / 255;
        uint8_t g = (tftG * currentBrightness) / 255;
        uint8_t b = (tftB * currentBrightness) / 255;
        
        uint16_t blockColor = tft.color565(r, g, b); 
        
        if (currentBrightness == 0) {
          blockColor = tft.color565(40, 40, 40); // 꺼졌을 때는 기본 짙은 회색
        }
        
        tft.fillRect(startX + (x * (blockSize + gap)), 
                     startY + (y * (blockSize + gap)), 
                     blockSize, blockSize, blockColor);
      }
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
          DEBUG_SERIAL.println("--- 2 Seconds Reached! Controlling Gripper Immediate ---");
          longPressExecuted = true; 
      }
  }
  if (myBtn.wasReleased()) {
      if (longPressExecuted) {
          longPressExecuted = false; 
          DEBUG_SERIAL.println("Button Released after Long Press (Gripper Done)");
      } else {
          DEBUG_SERIAL.println("--- Short Press Detected on Release: Controlling Pusher ---");
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