/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/solved-reconnect-esp32-to-wifi/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <EEPROM.h>
#define EEPROM_SIZE 128

// Replace with your network credentials (STATION)
//const char* ssid = "IT";
//const char* password = "@Polytech";
const char* ssid = "LDH";
const char* password = "ehdgml43";

char TEMP_SSID[32];
char TEMP_PW[32];

char EEPROM_SSID[32];
char EEPROM_PW[32];

unsigned long previousMillis = 0;
unsigned long interval = 30000;
int connection_flag = 0;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.print("connected ->");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  EEPROM.begin(EEPROM_SIZE);
  
  strcpy(TEMP_SSID, ssid);    // char* -> char[]
  strcpy(TEMP_PW, password);

  for(int i=0; i < sizeof(EEPROM_SSID); i++){
      EEPROM.write(i, 0);
      EEPROM.write(i + sizeof(EEPROM_SSID), 0);
      EEPROM.write(i, TEMP_SSID[i]);
      EEPROM.write(i + sizeof(EEPROM_SSID), TEMP_PW[i]);
  }
  
  EEPROM.commit();

  for(int j=0; j< sizeof(EEPROM_SSID); j++){
    EEPROM_SSID[j] = EEPROM.read(j);
    EEPROM_PW[j] = EEPROM.read(j + sizeof(EEPROM_SSID));
  }
  Serial.print("EEPROM SSID : ");
  Serial.println(EEPROM_SSID);
  Serial.print("EEPROM SSID : ");
  Serial.println(EEPROM_PW);

}

void loop() {
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
