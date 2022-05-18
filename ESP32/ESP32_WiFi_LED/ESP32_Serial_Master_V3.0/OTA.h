#ifndef OTA_h
#define OTA_h

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#endif


#include <Adafruit_NeoPixel.h>
#include <AsyncElegantOTA.h>
#include <TelnetStream.h>
#include "SPIFFS.h"
#include <WiFiManager.h>

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include "stdint.h"

AsyncWebServer server(80);


void init_Neopixel(uint8_t brightness);
void ota_handle( void * parameter );
void initWiFi();
void reconnectWiFi();
void initOLED();
void setupOTA();
void writeWiFiEEPROM();
void readWiFiEEPROM();
void showOLED_IP_Address();
void pickOneLED(uint8_t ledNum, uint8_t R, uint8_t G, uint8_t B, uint8_t brightness, uint8_t wait);
void blinkNeopixel(uint8_t R, uint8_t G, uint8_t B, int times);
void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void rainbow(int wait);
void theaterChaseRainbow(int wait);

#endif
