#ifndef OTA_h
#define OTA_h


#include <WiFi.h>
#include <AsyncTCP.h>
#include <TelnetStream.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>


void initWiFi();
void changeWiFi();
void reconnectWiFi();

#endif
