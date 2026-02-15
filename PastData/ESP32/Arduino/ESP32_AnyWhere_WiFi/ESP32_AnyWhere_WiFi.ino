#include "OTA.h"

#define EEPROM_SIZE 128



int cnt = 0;



void setup() {
  Serial.begin(115200);
  init_Neopixel(50);
  initOLED();
  EEPROM.begin(EEPROM_SIZE);
  setupOTA("ESP_LED");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

}

void loop() {
  reconnectWiFi();
  ArduinoOTA.handle();
  //pickOneLED(0, strip.Color(255,   0,   0), 50);
  rainbow(10);
  TelnetStream.println(cnt);
  cnt++;

 

}
