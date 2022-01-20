#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// for using Korean NanumGothicCoding font
#include "u8g2_font_unifont_t_korean_NanumGothicCoding_16.h"

// Please UNCOMMENT one of the contructor lines below
// U8g2 Contructor List (Frame Buffer)
// The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected

// for HW_SPI (VSPI)
//U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 14, /* reset=*/ 15);
// for SW_SPI
//U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 26, /* data=*/ 25, /* cs=*/ 5, /* dc=*/ 14, /* reset=*/ 15);
// for SSD1306, I2C, (ESP32: SDA 21, SCL 22)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// for SH1106,  I2C, (ESP32: SDA 21, SCL 22)
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup(void) 
{
  u8g2.begin();
  u8g2.enableUTF8Print();
}

void loop(void) 
{

  //u8g2.setFont(u8g2_font_unifont_t_korean1);
  //u8g2.setFont(u8g2_font_unifont_t_korean2);
  u8g2.setFont(u8g2_font_unifont_t_korean_NanumGothicCoding_16);  // 나눔고딕은 잘 안먹힘
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 16);
  u8g2.print("Hello!");
  u8g2.setCursor(0,40);
  u8g2.print("안녕.!");  
  u8g2.sendBuffer();
  
  delay(1000);
}
