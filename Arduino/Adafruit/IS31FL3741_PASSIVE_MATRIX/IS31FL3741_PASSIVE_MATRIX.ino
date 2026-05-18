#include <Adafruit_IS31FL3741.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <NimBLEDevice.h>
#include <SPI.h>

#include "Button.h"

#define DEBUG_SERIAL Serial

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


#define TFT_RST           1  // Or set to -1 and connect to Arduino RESET pin
#define TFT_CS            2  // TC Pin
#define SWITCH_PIN        3
#define TFT_DC            4
#define I2C_SDA_PIN       5
#define I2C_SCL_PIN       6
#define TFT_SCK           7  // Not Use in this code
#define MATRIX_SHTDN      8
#define TFT_MOSI          9 // Not Use in this code

#define UART_TX           43
#define UART_RX           44



const int MATRIX_WIDTH = 8;   // ROW
const int MATRIX_HEIGHT = 8;  // COLUMN
const float p = 3.1415926;

unsigned long ledMillis = 0;
unsigned long stateMillis = 0;
unsigned long tftUpdateMillis = 0;
unsigned long matrixUpdateMillis = 0;

Adafruit_IS31FL3741 ledMatrix;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCharacteristic = nullptr;
uint8_t ledBuffer[MATRIX_WIDTH * MATRIX_HEIGHT] = {0,};
bool currentBleConnected = false;  // 현재 연결 상태
bool lastBleConnected = false;     // 직전 루프의 연결 상태 (변경 감지용)

Button myBtn(SWITCH_PIN, true, 50);
bool longPressExecuted = false;

class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
      DEBUG_SERIAL.println("PC/스마트폰이 연결되었습니다! (NimBLE 최신 규격)");
      currentBleConnected = true;
      // 연결된 기기의 RSSI(신호세기) 등을 모니터링할 수도 있습니다.
  }
  
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
      DEBUG_SERIAL.println("연결 해제됨. 다시 광고 시작...");
      // 공식 예제 방식: 디바이스 전역 함수를 통해 광고 재시작
      currentBleConnected = false; // [수정] 연결됨 플래그 OFF
      NimBLEDevice::startAdvertising(); 
  }
};

class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        // 최신 버전에서 데이터를 안전하게 읽어오는 방식
        std::string value = pChar->getValue();
        
        if (value.length() == 64) {
            // 수신된 64바이트를 전역 ledBuffer에 즉시 복사
            memcpy(ledBuffer, value.data(), 64);
        } else {
            DEBUG_SERIAL.print("데이터 길이 불일치: ");
            DEBUG_SERIAL.print(value.length());
            DEBUG_SERIAL.println(" 바이트 수신됨. (64바이트여야 합니다)");
        }
    }
};

void setup() {
  DEBUG_SERIAL.begin(115200);

  tft.init(240, 280);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(20, 20);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Start LED Matrix");

  tft.setCursor(20, 40);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_RED);
  tft.print("BLE: DISCONNECTED");

  tft.drawRect(38, 68, 164, 164, ST77XX_WHITE);

  pinMode(LED_BUILTIN, OUTPUT); // GPIO21
  digitalWrite(LED_BUILTIN, LOW);  
  buttonInit(SWITCH_PIN);

  Serial.println("I2C Bus Init...");
  if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
    Serial.println("I2C Init Failed");
  }
  else{
    if(!i2cCheckSingleShot()){
      Serial.println("경고: 연결된 I2C 장치가 없습니다!");
    }
  }

  for(int i=0; i<10; i++){
    // I2C 기본 주소로 칩 초기화
    if (!ledMatrix.begin(IS3741_ADDR_DEFAULT, &Wire)) {
      DEBUG_SERIAL.print("[");
      DEBUG_SERIAL.print(i + 1);
      DEBUG_SERIAL.println("] IS31FL3741 칩을 찾을 수 없습니다. 재시도 중... ");
      delay(100);
    }
    else{
      Serial.println("IS31FL3741 초기화 성공!");
      Wire.setClock(800000);
      ledMatrix.setLEDscaling(0xFF);
      ledMatrix.setGlobalCurrent(0xFF);
      ledMatrix.enable(true); 
      ledMatrix.fill(0);
      break;
    }
  }

  initBLE();
  
  DEBUG_SERIAL.println("SETUP DONE");
}

void loop() {
  unsigned long curMillis = millis();
  buttonTask();
  // ledTestNonBlocking(50);
  if (curMillis - matrixUpdateMillis >= 30) {
    matrixUpdateMillis = curMillis;
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      for (int x = 0; x < MATRIX_WIDTH; x++) {
        uint8_t brightness = ledBuffer[(y * MATRIX_WIDTH) + x];
        drawMonoPixel(x, y, brightness);
      }
    }
  }


  
  if (curMillis - tftUpdateMillis >= 150) {
    tftUpdateMillis = curMillis;
    updateTftDisplay();
  }

  if (currentBleConnected != lastBleConnected) {
    lastBleConnected = currentBleConnected; // 상태 동기화

    // 기존 글씨 찌꺼기를 지우기 위해 상태창 영역(상단 우측)만 검은색 사각형으로 지우기
    // 가로 240, 세로 280 해상도 기준 우측 상단 레이아웃 지정 (x: 20~220 영역 뒤쪽)
    tft.fillRect(20, 40, 200, 20, ST77XX_BLACK); 

    tft.setCursor(20, 40);
    tft.setTextSize(1);

    if (currentBleConnected) {
      tft.setTextColor(ST77XX_BLUE);
      tft.print("BLE: CONNECTED");
      DEBUG_SERIAL.println("TFT Display Update: CONNECTED");
    } else {
      tft.setTextColor(ST77XX_RED);
      tft.print("BLE: DISCONNECTED");
      DEBUG_SERIAL.println("TFT Display Update: DISCONNECTED");
    }
  }


  if(curMillis - ledMillis >= 10){
    ledMillis = curMillis;
    
  }
  if(curMillis - stateMillis >= 1000){
    stateMillis = curMillis;

  }
  breath(5);
}

void updateTftDisplay() {
  int startX = 40;  // 매트릭스가 그려질 디스플레이 내부 시작 X 좌표
  int startY = 70; // 매트릭스가 그려질 디스플레이 내부 시작 Y 좌표
  int blockSize = 18; // 각 LED 소자를 표현할 사각형 크기 (픽셀 단위)
  int gap = 2;        // 사각형 격자 사이의 간격

  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      uint8_t brightness = ledBuffer[(y * MATRIX_WIDTH) + x];
      
      // 단색 밝기(0~255) 값을 고대비 컬러 모니터링용 색상으로 가공 (565 포맷 포지셔닝)
      // 여기서는 밝기가 밝을수록 강한 초록빛(Green)을 띠도록 변환합니다.
      uint16_t blockColor = tft.color565(0, brightness, 0); 
      
      // 만약 밝기가 0(꺼짐)이라면 격자 그리드가 보이기 좋게 어두운 회색으로 표현합니다.
      if (brightness == 0) {
        blockColor = tft.color565(40, 40, 40); 
      }

      // 지정된 픽셀 좌표에 데이터 사각형 그리기
      tft.fillRect(startX + (x * (blockSize + gap)), 
                   startY + (y * (blockSize + gap)), 
                   blockSize, blockSize, blockColor);
    }
  }
}


void initBLE(){
  NimBLEDevice::init("ESP32_NimBLE_MATRIX");

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  
  // 고속 스트리밍을 위한 Write Without Response(WRITE_NR) 설정
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
  pAdvertising->start();

  DEBUG_SERIAL.println("NimBLE 최신 서버 활성화 완료! 검색 가능합니다.");
}



void drawMonoPixel(int16_t x, int16_t y, uint8_t brightness) {
  // 매트릭스 경계 검사 (안전장치)
  if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
    
    // 2차원 좌표를 1차원 선형 LED 번호(0 ~ 350)로 변환하는 공식
    uint16_t lednum = (y * MATRIX_WIDTH) + x;
    
    // 칩이 지원하는 최대 채널 수 검사
    if (lednum < 351) {
      // 로우레벨 기본 함수로 특정 번호의 단색 LED 밝기 직접 제어
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
  // static bool longPressExecuted = false;
  myBtn.read();

  if (myBtn.pressedFor(2000)) {
      if (!longPressExecuted) {
          DEBUG_SERIAL.println("--- 2 Seconds Reached! Controlling Gripper Immediate ---");
              
          longPressExecuted = true; // 문 걸어잠그기
      }
  }
  if (myBtn.wasReleased()) {
      if (longPressExecuted) {
          longPressExecuted = false; 
          DEBUG_SERIAL.println("Button Released after Long Press (Gripper Done)");
      } 
      // 2초가 되기 전에 손을 뗐다면 -> "짧은 누름"이므로 푸셔(Pusher) 구동
      else {
          DEBUG_SERIAL.println("--- Short Press Detected on Release: Controlling Pusher ---");
        
      }
  }
}

void ledTestNonBlocking(const unsigned long pixelInterval) {
  // 함수 내부에서만 접근 가능하지만, 값은 메모리에 계속 유지되는 static 변수
  static int currentX = 0;
  static int currentY = 0;
  static unsigned long lastPixelTime = 0;

  unsigned long currentMillis = millis();

  // 입력받은 const 인자값(pixelInterval)으로 시간 체크
  if (currentMillis - lastPixelTime >= pixelInterval) {
    lastPixelTime = currentMillis; // 타이머 리셋

    // 1. 현재 켜져 있는 픽셀 끄기
    drawMonoPixel(currentX, currentY, 0);

    // 2. 다음 픽셀 위치로 좌표 이동
    currentX++; 
    
    if (currentX >= MATRIX_WIDTH) { 
      currentX = 0;                
      currentY++;                  
      
      if (currentY >= MATRIX_HEIGHT) { 
        currentY = 0;                  
      }
    }

    // 3. 새로 이동한 위치의 픽셀 켜기
    drawMonoPixel(currentX, currentY, 255);
  }
}


// i2C 주소 확인용
bool i2cCheck(int interval) {
  static byte address = 1;
  static int devicesFound = 0;
  static unsigned long lastActionTime = 0;
  static bool isWaitingNextCycle = false;

  unsigned long currentTime = millis();

  if (isWaitingNextCycle) {
    if (currentTime - lastActionTime >= (unsigned long)interval) {
      isWaitingNextCycle = false; 
      address = 1;               
      devicesFound = 0;
      Serial.println("\n--- 새 스캔 시작 ---");
    }
    return false; 
  }

  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();

  if (error == 0) {
    Serial.print("Found: 0x");
    if (address < 16) Serial.print("0");
    Serial.println(address, HEX);
    devicesFound++;
  }

  address++;
  if (address >= 127) {
    bool result = (devicesFound > 0);
    lastActionTime = currentTime; // 완료 시점 기록
    isWaitingNextCycle = true;    // 대기 모드 진입

    Serial.print("Scan Done. Found: ");
    Serial.println(devicesFound);
    return true; 
  }

  return false;
}


// setup()에서 한 번만 호출하여 I2C 디바이스를 완벽히 스캔하는 함수
bool i2cCheckSingleShot() {
  int devicesFound = 0;
  
  Serial.println("\n--- I2C 스캔 시작 (setup) ---");

  // 1번부터 126번 주소까지 한 번에 쭉 돌아버립니다.
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Found: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devicesFound++;
    }
    // 주소 간 아주 미세한 안정화 대기 (필요시 1~2ms 추가 가능, 생략 무방)
    delay(1); 
  }

  Serial.print("Scan Done. Total Found: ");
  Serial.println(devicesFound);
  Serial.println("-----------------------------\n");

  // 장치를 하나라도 찾았다면 true, 하나도 없다면 false 반환
  return (devicesFound > 0);
}

void breath(int interval) {
  static int breathBrightness = 0; // 현재 밝기 (0 ~ 255)
  static int breathAmount = 1;     // 한 번에 변화할 밝기 양 (양수면 밝아짐, 음수면 어두워짐)
  unsigned long lastBreathTime = 0; // 마지막으로 밝기를 바꾼 시간 저장


  unsigned long currentMillis = millis();

  // 입력받은 interval(ms)마다 밝기를 1단계씩 변화시킵니다.
  if (currentMillis - lastBreathTime >= (unsigned long)interval) {
    lastBreathTime = currentMillis; // 타이머 리셋

    // 현재 밝기 적용 (ESP32-S3 내장 LED 구동)
    analogWrite(LED_BUILTIN, breathBrightness);

    // 다음 루프를 위한 밝기 계산
    breathBrightness += breathAmount;

    // 밝기가 최댓값(255)에 도달하면 어두워지도록 방향 전환
    if (breathBrightness >= 255) {
      breathBrightness = 255;
      breathAmount = -1; // 감소 방향으로
    }
    // 밝기가 최솟값(0)에 도달하면 다시 밝아지도록 방향 전환
    else if (breathBrightness <= 0) {
      breathBrightness = 0;
      breathAmount = 1;  // 증가 방향으로
    }
  }
}
