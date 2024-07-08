#include <Arduino.h>
#include "MyWiFi.h"
#include "MyOTA.h"

char apName[] = "WATER_XIAO";

AsyncWebServer server(80);
MyWiFi* myWiFi = new MyWiFi(&server);
MyOTA* myOTA = new MyOTA();

void setup() {
  Serial.begin(115200);
  Serial.println(WiFi.localIP());
  myWiFi->beginWiFi(apName);


  myOTA->initOTA(&server, true);


}

uint32_t count = 0;
unsigned long countTime = 0;
void loop() {
  
  if(millis() - countTime > 2000)
  {
    countTime = millis();
    Serial.printf("COUNT : %d\r\n", count);
  }

  myOTA->loopOTA();
}

