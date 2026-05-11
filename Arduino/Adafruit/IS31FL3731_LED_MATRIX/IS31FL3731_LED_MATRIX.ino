// TargetBoard: Adafruit EAdafruit QT Py ESP32 Pico (Product ID: 5395)
// Board Link
// https://learn.adafruit.com/adafruit-qt-py-esp32-pico/pinouts

// LED Matrix Driver : Charlieplexed PWM LED Matrix Driver - IS31FL3731 (Product ID: 2946)
// PWM LED Driver Link
// https://learn.adafruit.com/i31fl3731-16x9-charliplexed-pwm-led-driver/pinouts

// Library
// https://github.com/adafruit/Adafruit_IS31FL3731
// https://github.com/adafruit/adafruit_neopixel
// https://github.com/adafruit/Adafruit-gfx-library

// LED Array Rule : 참고용(실사용X)
// C1-1: A2->(A1), C1-2: A3->(A1) etc. [Start with->Except A1 (A2,A3,A4..)]/[Ended to A1] -> Cathode A1 
// C2-1: A1->(A2), C2-2: A3->(A2) etc. [Start with->Except A2 (A1,A3,A4..)]/[Ended to A2] -> Cathode A2
// C3-1: A1->(A3), C3-2: A2->(A3) etc. [Start with->Except A3 (A1,A2,A4..)]/[Ended to A3] -> Cathode A3
// A1 Anode ->     , C2-1, C3-1, C4-1, C5-1, C6-1, C7-1, C8-1, C9-1
// A2 Anode -> C1-1,     , C3-2, C4-2, C5-2, C6-2, C7-2, C8-2, C9-2
// A3 Anode -> C1-2, C2-2,     , C4-3, C5-3, C6-3, C7-3, C8-3, C9-3
// A4 Anode -> C1-3, C2-3, C3-3,     , C5-4, C6-4, C7-4, C8-4, C9-4
// A5 Anode -> C1-4, C2-4, C3-4, C4-4,     , C6-5, C7-5, C8-5, C9-5
// A6 Anode -> C1-5, C2-5, C3-5, C4-5, C5-5,     , C7-6, C8-6, C9-6
// A7 Anode -> C1-6, C2-6, C3-6, C4-6, C5-6, C6-6,     , C8-7, C9-7
// A8 Anode -> C1-7, C2-7, C3-7, C4-7, C5-7, C6-7, C7-7,     , C9-8
// A9 Anode -> C1-8, C2-8, C3-8, C4-8, C5-8, C6-8, C7-8, C8-8, 

// 예시: C2-3 (Row 2 / Column 3 배치)  Anode A4 -> Cathode A2 
// Anode -> 위 배치 표를 보고 선택
// Cathode -> Row 를 보고 선택

/////////////////////////////////////////////////////////////
// 8x8 Matrix 제작시 A1 ~ A8, B1~B8 까지 사용
// ITO Pattern 에 대응
// Scan Line (Row, Sink) : B1 ~ B8
// Data Line (Column, Source) : A1 ~ A8
// 예시: C2-3 을 제어하고 싶으면 A2 -> B3
/////////////////////////////////////////////////////////////


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_NeoPixel.h>

// #define DEBUG

#define LED_PIN PIN_NEOPIXEL // PIN NUMBER 5
#define NUM_PIXELS 1
#define I2C_SDA_PIN 22
#define I2C_SCL_PIN 19
#define I2C_ADDR 0x74
#define FD_REG_CONF 0x00      // 설정 레지스터
#define FD_REG_PWM_START 0x24 // PWM 레지스터 시작 (Frame 0)

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();  // Default I2C Address 0x74

int count = 0;
unsigned long prevMillis = 0;
uint8_t ledValue = 0;
bool ledDir = false;

void setup() {
  Serial.begin(115200);
  pixels.begin();
  
  Serial.println("I2C Bus Init...");
  if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
    Serial.println("I2C Init Failed");
  }

  pixels.clear();
  pixels.setBrightness(10);
  pixels.setPixelColor(0, pixels.Color(100, 0, 0));
  pixels.show();

  for(int i=0; i<10; i++){
    if(!ledmatrix.begin()){
      Serial.println("IS31 not found");
    }else{
      Serial.println("IS31 Found!");
      pixels.setPixelColor(0, pixels.Color(0, 100, 0));
      pixels.show();
      break;
    }
    delay(500);
  }

}


void loop() {

  unsigned long currMillis = millis();
  if(currMillis - prevMillis >= 10){
    prevMillis = currMillis;
    
    if(ledValue >= 50)
    {
      ledDir = false;
    }
    else if(ledValue <= 0)
    {
      ledDir = true;
    }

    if(ledDir)
    {
      ledValue++;
    }
    else
    {
      ledValue--;
    }
    // ledmatrix.drawPixel(0, 0, ledValue);
    myDrawPixel(0,0, ledValue);
    // Serial.printf("LED Value: %d\r\n", ledValue);
  }



#ifdef DEBUG
  if (i2cCheck(1000)) {
    Serial.printf("I2C Cycle: %d\r\n",count++);
  }
#endif

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

void myDrawPixel(uint8_t x, uint8_t y, uint8_t brightness) {
  if (x > 7 || y > 7) return;

  // IS31FL3731의 Matrix B(CB핀) PWM 레지스터는 0x24부터 시작하며 
  // 데이터시트 매핑에 따라 특정 규칙을 가집니다.
  // A핀(Source)과 B핀(Sink) 사이의 매핑 주소 계산:
  // Matrix B의 경우, 각 Row(CB)는 16바이트 간격으로 배치되는 경우가 많습니다.
  
  uint8_t regAddr = FD_REG_PWM_START + (y * 16) + x; 

  writeRegister(0xFD, 0x00); // Frame 0 확인
  writeRegister(regAddr, brightness);
}

void writeRegister(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}
