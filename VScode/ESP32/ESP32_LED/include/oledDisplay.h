
#ifndef __OLED__H_
#define __OLED__H_

#define SDA_PIN 21
#define SCL_PIN 22

#include <Wire.h>
#include <U8g2lib.h>

extern U8G2 u8g2_single;

class oledDisplay
{
private:
    

public:

    oledDisplay();
    oledDisplay(int sda, int scl);
    ~oledDisplay();

    void oledDisplayWiFi();
    void oledLightStatus();
};





#endif