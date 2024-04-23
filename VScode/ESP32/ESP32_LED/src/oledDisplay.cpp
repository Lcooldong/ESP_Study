#include "oledDisplay.h"

U8G2 u8g2_single;

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2_hw(U8G2_R0, U8X8_PIN_NONE);


oledDisplay::oledDisplay()
{
    u8g2_single = u8g2_hw;

    u8g2_single.begin();

}


oledDisplay::oledDisplay(int sda, int scl)
{
    U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2_sw(U8G2_R0, scl, sda, U8X8_PIN_NONE);
    u8g2_single = u8g2_sw;

    u8g2_single.begin();
}

oledDisplay::~oledDisplay()
{

}

void oledDisplay::oledDisplayWiFi()
{


}

void oledDisplay::oledLightStatus()
{

}