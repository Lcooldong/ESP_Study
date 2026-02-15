#include "FS.h"  // ESP8266 SPIFF 라이브러리
//#include "SPIFFS.h"  // ESP32 SPIFSPIFFS 라이브러리

void setup() {
  Serial.begin(115200);
  Serial.println();
  if(SPIFFS.begin())  Serial.println("SPIFFS Initialize....ok"); //Initialize File System
  else  Serial.println("SPIFFS Initialization...failed");
  if(SPIFFS.format())  Serial.println("File System Formated");  //Format File System
  else   Serial.println("File System Formatting Error");
}

void loop() {
  // put your main code here, to run repeatedly:

}
