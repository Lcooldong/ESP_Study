#include "MyOTA.h"
#include <Arduino.h>

unsigned long ota_progress_millis = 0;

MyOTA::MyOTA()
{

}

MyOTA::~MyOTA()
{

}



void MyOTA::onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void MyOTA::onOTAProgress(unsigned int current, unsigned int final) {
  // Log every 1 second
  
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\r\n", current, final);
  }
}

void MyOTA::onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

// server.begin() : true
void MyOTA::initOTA(AsyncWebServer* _server, bool _serverStart)
{
  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is ElegantOTA AsyncDemo.");
  });

  ElegantOTA.begin(_server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  
  if(_serverStart)
  {
    _server->begin();
  }
}

void MyOTA::loopOTA()
{
    ElegantOTA.loop();
}