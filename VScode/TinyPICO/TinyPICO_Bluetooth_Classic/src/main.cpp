#include <Arduino.h>
#include <SPI.h>
#include <BluetoothSerial.h>
#include "TinyPICO.h"

BluetoothSerial serialBT;
char cmd;
TinyPICO tp = TinyPICO();


void setup() {
  tp.DotStar_SetPixelColor(0xFF0000);
  tp.DotStar_Show();
  serialBT.begin("TinyPICO-Bluetooth");
  

}

void loop() {
  if(serialBT.available())
  {
    cmd = serialBT.read();
    tp.DotStar_SetPixelColor(0x0000FF);
    tp.DotStar_Show();
  }
  
  delay(10);
}
