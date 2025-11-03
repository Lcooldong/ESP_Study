#ifndef ARDUINO_MYWiFi_H
#define ARDUINO_MYWiFi_H

#include <Arduino.h>


void beginWiFi(String apName = "ESP32");
void init_WiFi();
void beginWiFiManager(String apName);

#endif