#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WebSerial.h>
#include <ESPAsyncWiFiManager.h>
#include <ArduinoOTA.h>
#include "MyLittleFS.h"


#define FIXED_IP
#define WIFI_CONNECTION_INTERVAL 10000

void setUpWiFi();
void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);
void recvMsg(uint8_t *data, size_t len);
void arduinoOTAProgress();