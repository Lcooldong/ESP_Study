#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define LED_COUNT 1
#define LED_PIN 2

class MyNeopixel{


public:

    Adafruit_NeoPixel* strip;
    MyNeopixel()
    {   
        //strip 
        strip = new Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
        InitNeopixel();
        
    }    


    //생성자
    MyNeopixel(uint16_t pinCnt, uint16_t pinNum)
    {   
        strip = new Adafruit_NeoPixel(pinCnt, pinNum, NEO_GRB + NEO_KHZ800);
        InitNeopixel();
        
    }

    // 소멸자
    ~MyNeopixel()
    {

    }

    void InitNeopixel();
    void pickOneLED(uint8_t ledNum, uint32_t color, uint8_t brightness, int wait);
    void blinkNeopixel(uint32_t color, int times, int delays);
    void resetNeopixel();
    void colorWipe(uint32_t c, uint8_t wait);
    void rainbow(uint8_t wait);
    void rainbowCycle(uint8_t wait);
    void theaterChase(uint32_t c, uint8_t wait) ;
    void theaterChaseRainbow(uint8_t wait);
    uint32_t Wheel(byte WheelPos);

private:

};


#endif