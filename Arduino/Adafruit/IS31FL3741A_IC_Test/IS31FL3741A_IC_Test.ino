#include <Adafruit_IS31FL3741.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include "Button.h"

#define DEBUG_SERIAL Serial


// PIN FOR Seeed XIAO 
#define TFT_RST           D0  // Or set to -1 and connect to Arduino RESET pin
#define TFT_CS            D1  // TC Pin
#define SWITCH_PIN        D2
#define TFT_DC            D3
#define I2C_SDA_PIN       D4
#define I2C_SCL_PIN       D5
#define TFT_SCK           D8   // Not Use in this code
#define MATRIX_SHTDN      D9
#define TFT_MOSI          D10 // Not Use in this code

#define UART_TX           D6
#define UART_RX           D7

const int MATRIX_WIDTH = 8;   // ROW
const int MATRIX_HEIGHT = 8;  // COLUMN
const float p = 3.1415926;

unsigned long ledMillis = 0;
unsigned long stateMillis = 0;
unsigned long matrixUpdateMillis = 0;

const uint16_t ledMap[8][8] = {
  // x: CS1~CS8, y: SW1
  {  0,  1,  2,  3,  4,  5,  6,  7 },
  // y: SW2
  { 39, 40, 41, 42, 43, 44, 45, 46 },
  // y: SW3
  { 78, 79, 80, 81, 82, 83, 84, 85 },
  // y: SW4
  {117,118,119,120,121,122,123,124 },
  // y: SW5
  {156,157,158,159,160,161,162,163 },
  // y: SW6
  {195,196,197,198,199,200,201,202 },
  // y: SW7
  {234,235,236,237,238,239,240,241 },
  // y: SW8
  {273,274,275,276,277,278,279,280 }
};

Adafruit_IS31FL3741 ledMatrix;
uint8_t ledBuffer[MATRIX_WIDTH * MATRIX_HEIGHT] = {0,};

Button myBtn(SWITCH_PIN, true, 50);
bool longPressExecuted = false;


void setup() {
  DEBUG_SERIAL.begin(115200);
  
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
      Wire.setClock(400000); 
      
      ledMatrix.setGlobalCurrent(0x40);  // 처음에는 너무 높이지 말고 0x20~0x80 권장
      ledMatrix.setLEDscaling(0xFF);
      ledMatrix.fill(0);
      ledMatrix.enable(true);      // 칩 활성화
      break;
    }
  }

  DEBUG_SERIAL.println("SETUP DONE");



}

void loop() {
  // ledTestNonBlocking(500);
  scanRawLedNum(0, 350, 30, 300);

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
    drawMonoPixel(currentX, currentY, 10);
  }
}

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

void drawMonoPixel(int16_t x, int16_t y, uint8_t brightness) {
  if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) return;

  uint16_t lednum = ledMap[y][x];

  bool ok = ledMatrix.setLEDPWM(lednum, brightness);

  if (!ok) {
    Serial.print("setLEDPWM failed: ");
    Serial.println(lednum);
  }
}

void scanRawLedNum(uint16_t start, uint16_t end, uint8_t brightness, unsigned long intervalMs) {
  static uint16_t lednum = start;
  static uint16_t prev = start;
  static unsigned long lastTime = 0;

  if (millis() - lastTime < intervalMs) return;
  lastTime = millis();

  ledMatrix.setLEDPWM(prev, 0);

  Serial.print("LEDNUM = ");
  Serial.println(lednum);

  ledMatrix.setLEDPWM(lednum, brightness);
  prev = lednum;

  lednum++;
  if (lednum > end) lednum = start;
}
