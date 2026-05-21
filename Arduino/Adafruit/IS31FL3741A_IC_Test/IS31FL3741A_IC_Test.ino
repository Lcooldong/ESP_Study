#include <Adafruit_IS31FL3741.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "Button.h"

#define DEBUG_SERIAL Serial

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
unsigned long matrixUpdateMillis = 0;

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
      Wire.setClock(800000);
      ledMatrix.setLEDscaling(0xFF);
      ledMatrix.setGlobalCurrent(0xFF);
      ledMatrix.enable(true); 
      ledMatrix.fill(0);
      break;
    }
  }

  DEBUG_SERIAL.println("SETUP DONE");
}

void loop() {
  ledTestNonBlocking(100);

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
