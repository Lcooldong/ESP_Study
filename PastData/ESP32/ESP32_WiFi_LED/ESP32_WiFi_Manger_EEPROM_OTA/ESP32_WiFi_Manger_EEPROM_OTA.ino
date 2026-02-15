#include "OTA.h"
#define ESP32_RTOS


unsigned long previousMillis = 0;
unsigned long interval = 30000;
int connection_flag = 0;

int cnt = 0;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  setupOTA("ESP_LED");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

}

void loop() {
  reconnectWiFi();
  ArduinoOTA.handle();
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
