#ifndef OLED_h
#define OLED_h

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void initOLED();
void showOLED_IP_Address();
void showOLED_WiFi(char* ssid, char* pass);

#endif
