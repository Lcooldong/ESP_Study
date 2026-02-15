#include "OTA.h"
#define EEPROM_SIZE 128


unsigned long previousMillis = 0;
unsigned long interval = 30000;
int connection_flag = 0;

int cnt = 0;

void setup() {
  Serial.begin(115200);
  initOLED();
  EEPROM.begin(EEPROM_SIZE);
  setupOTA("ESP_LED");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  init_Neopixel(50);
}

void loop() {
  reconnectWiFi();
  ArduinoOTA.handle();
  //pickOneLED(0, strip.Color(255,   0,   0), 50);
  rainbow(10);
  TelnetStream.println(cnt);
  cnt++;
  delay(500);
}



void reconnectWiFi(){
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.println(millis());
    Serial.println("Reconnecting to WiFi...");
    
    WiFi.disconnect();
    WiFi.reconnect();
    connection_flag = 1;
    previousMillis = currentMillis;
  }
  else if(connection_flag == 1)
  {
    Serial.println("reconnected");  
    connection_flag = 0;
  }
}
