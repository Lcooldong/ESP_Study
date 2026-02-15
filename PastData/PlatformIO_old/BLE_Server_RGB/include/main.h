#pragma once

void parcing_rgb(String RGB);
void neopixel_init(void);
void pickOneLED(uint8_t ledNum, uint32_t color, uint8_t brightness, int wait);
void blinkNeopixel(uint32_t color, int times, int delays);
void resetNeopixel(void);
void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void rainbow(uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);