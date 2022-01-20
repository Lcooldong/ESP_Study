#include <Arduino.h>
#include <U8g2lib.h>

// for SPI
#include <SPI.h>

// for using Korean NanumGothicCoding font
//#include "u8g2_font_unifont_t_korean_NanumGothicCoding_16.h"

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

void setup() {
  u8g2.begin();
  u8g2.enableUTF8Print();   // enable UTF8 support for the Arduino print() function
}

void loop() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(0,15, "SKS STEAM!");
    u8g2.drawTriangle(27,50, 64,32, 100,50);
  } while ( u8g2.nextPage() );
  delay(1000);
}
