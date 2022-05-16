#ifndef OTA_h
#define OTA_h

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#ifdef ESP32
  #include <WiFi.h>
  #include <ESPmDNS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#endif

#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include <TelnetStream.h>
#include <EEPROM.h>
#include <WiFiManager.h>

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include "stdint.h"

void init_Neopixel(uint8_t brightness);
void ota_handle( void * parameter );
void initWiFi();
void initOLED();
void setupOTA(const char* nameprefix);
void writeWiFiEEPROM();
void readWiFiEEPROM();
void showOLED_IP_Address();
void pickOneLED(uint8_t ledNum, uint32_t color, int wait);
void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void rainbow(int wait);
void theaterChaseRainbow(int wait);

#endif
