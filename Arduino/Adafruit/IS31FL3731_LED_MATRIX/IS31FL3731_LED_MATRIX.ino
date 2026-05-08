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

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_NeoPixel.h>

#define DEBUG

#define LED_PIN PIN_NEOPIXEL // PIN NUMBER 5
#define NUM_PIXELS 1
#define I2C_SDA_PIN 22
#define I2C_SCL_PIN 19

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();  // Default I2C Address 0x74
int count = 0;


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
  if (i2cCheck(1000)) {
    Serial.printf("I2C Cycle: %d\r\n",count++);
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
