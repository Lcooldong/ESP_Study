#ifndef OLED_h
#define OLED_h

#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>

void initOLED();
void u8g2_prepare(void);
void showOLED_IP_Address();
void showOLED_WiFi(char* ssid, char* pass);
void showOLED_changing_WiFi();
#endif
