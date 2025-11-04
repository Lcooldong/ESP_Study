#include <Arduino.h>
#include "unit_byte.hpp"
#include <M5Unified.h>


UnitByte device;

const int SDA_PIN = G2;
const int SCL_PIN = G1;
uint8_t buttonId = 0x47;
const uint32_t colors[] = {
    0xFF0000, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFFFFF, 0xFFA500, 0x808080, 0x00FF00,
};

void scanI2c();
void byteButton_Init();
void byteButton_Test();



void setup() {
  M5.begin();
  M5.Display.setEpdMode(epd_mode_t::epd_fastest);
  if(M5.Display.width() < M5.Display.height()) {
    M5.Display.setRotation(M5.Display.getRotation() ^ 1);
  }
  
  Serial.begin(115200); 
  delay(2000);
  Serial.println("Setup started.");

  scanI2c();
  byteButton_Init();


}

void loop() {
  M5.update();
  M5.Display.startWrite();
  
  static unsigned long lastMillis = 0;  
  if (millis() - lastMillis >= 100) {
    lastMillis = millis();
    // Serial.printf("Switch Status:%d\n", device.getSwitchStatus());  // 0-255 for 8 switches
    for (uint8_t i = 0; i < 8; i++) {
      uint8_t switchStatus8Bytes = device.getSwitchStatus(i);
      Serial.print("Switch Status Byte ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(switchStatus8Bytes);
      M5.Display.setCursor(4 + i*10, 20);
      M5.Display.setTextSize(1);
      M5.Display.printf("%d", switchStatus8Bytes);
    }
  }

  int state = M5.BtnA.wasHold() ? 1
            : M5.BtnA.wasClicked() ? 2
            : M5.BtnA.wasPressed() ? 3
            : M5.BtnA.wasReleased() ? 4
            : M5.BtnA.wasDecideClickCount() ? 5
            : 0;
  if( state ) {
    M5.Display.setCursor(4, 0);
    M5.Display.setTextSize(2);
    M5.Display.printf("BtnA: %d", state);
  }
  M5.Display.endWrite();

}


void scanI2c()
{
  Wire.begin();

  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
  
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(1000);           // wait 5 seconds for next scan
}

void byteButton_Init()
{
  if( !device.begin(&Wire, buttonId, SDA_PIN, SCL_PIN, 400000) ) {
      Serial.println("Unit Byte Button not found.");
  }
  else
  {
    Serial.println("Unit Byte Button found.");
  }

  device.setIRQEnable(false);
  
  device.setLEDShowMode(BYTE_LED_MODE_DEFAULT);
  Serial.println("Set LED show sys define.");

  device.setRGB233(8, colors[1]);
  device.setFlashWriteBack();
  
  for (int i = 0; i < 8; i++) {
      device.setSwitchOffRGB888(i, colors[i]);
      // Output the hexadecimal value of the current color
      Serial.printf("Set Switch Off RGB to %06X\n", (unsigned int)colors[i]);
  }
  for (int i = 0; i < 8; i++) {
      device.setSwitchOnRGB888(i, colors[9 - i]);
      // Output the hexadecimal value of the current color
      Serial.printf("Set Switch On RGB to %06X\n", (unsigned int)colors[i]);
  }
  device.setFlashWriteBack();
}


void byteButton_Test()
{
  if( !device.begin(&Wire, buttonId, SDA_PIN, SCL_PIN, 400000) ) {
      Serial.println("Unit Byte Button not found.");
  }
  else
  {
    Serial.println("Unit Byte Button found.");
  }

  Serial.printf("getLEDMode :%d\n", device.getLEDShowMode());
  delay(1000);
  Serial.printf("Switch Status:%d\n", device.getSwitchStatus());
  delay(1000);
  for (uint8_t i = 0; i < 8; i++) {
      uint8_t switchStatus8Bytes = device.getSwitchStatus(i);
      Serial.print("Switch Status Byte ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(switchStatus8Bytes);
  }
  delay(1000);

  device.setLEDShowMode(BYTE_LED_USER_DEFINED);
  Serial.println("Set LED show self define.");
  delay(1000);
  // Set LED brightness (assuming 250)
  for (uint8_t i = 0; i <= 8; i++) {
      device.setLEDBrightness(i, 250);
  }
  delay(1000);
  // Get LED brightness to confirm
  Serial.printf("New LED Brightness:: %02X\n", device.getLEDBrightness());
  delay(1000);
  // RGB
  for (int i = 0; i <= 8; i++) {
      device.setRGB888(i, colors[i]);
      // device.setRGB233(i, colors[8-i]);
      // Output the hexadecimal value of the current color
      Serial.printf("Set RGB to %06X\n", (unsigned int)colors[i]);
  }
  delay(1000);

  for (int i = 0; i < 8; i++) {
      // Serial.printf("Retrieved  RGB: %06X\n",device.getRGB888(i));
      Serial.printf("Retrieved  RGB: %06X\n", device.getRGB233(i));
  }

  delay(1000);
  device.setIRQEnable(false);
  delay(1000);
  Serial.printf("getIRQEnable: %02x\n", device.getIRQEnable());
  delay(1000);
  Serial.printf("Firmware Version: %02x\n", device.getFirmwareVersion());
  delay(1000);

  Serial.printf("read I2C Address:%02X\n", device.getI2CAddress());
  delay(1000);

  device.setI2CAddress(0x12);
  delay(1000);
  Serial.printf("new I2C Address: %02x\n", device.getI2CAddress());
  delay(2000);
  device.setI2CAddress(buttonId);
  delay(1000);

  device.setLEDShowMode(BYTE_LED_MODE_DEFAULT);
  Serial.println("Set LED show sys define.");
  delay(1000);
  device.setRGB233(8, colors[1]);
  delay(1000);
  device.setFlashWriteBack();
  delay(1000);
  for (int i = 0; i < 8; i++) {
      device.setSwitchOffRGB888(i, colors[i]);
      // Output the hexadecimal value of the current color
      Serial.printf("Set Switch Off RGB to %06X\n", (unsigned int)colors[i]);
  }
  delay(1000);
  for (int i = 0; i < 8; i++) {
      device.setSwitchOnRGB888(i, colors[9 - i]);
      // Output the hexadecimal value of the current color
      Serial.printf("Set Switch On RGB to %06X\n", (unsigned int)colors[i]);
  }
  delay(1000);
  device.setFlashWriteBack();
}