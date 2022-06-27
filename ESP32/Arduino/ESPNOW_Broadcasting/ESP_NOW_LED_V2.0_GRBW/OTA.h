#ifndef OTA_h
#define OTA_h

#define SDA_PIN 5
#define SCL_PIN 6


#include <AsyncTCP.h>
#include <TelnetStream.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>


void initWiFi();
void changeWiFi();
void reconnectWiFi();
void blinkTimer(uint32_t color);

#endif
