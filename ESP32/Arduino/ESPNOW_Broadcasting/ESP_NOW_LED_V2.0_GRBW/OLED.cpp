

#include "OLED.h"

//#include <WiFi.h>


U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.42" OLED

void initOLED(){
  u8g2.begin();
  
}

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void showOLED_IP_Address(){
  u8g2.clearBuffer();
  u8g2_prepare();
  u8g2.setCursor(0, 15);
//  u8g2.print(WiFi.localIP());
  u8g2.sendBuffer();
}


void showOLED_WiFi(char* ssid, char* pass){
  u8g2.clearBuffer();
  u8g2_prepare();
  
  u8g2.setCursor(0, 30);
  u8g2.print(ssid);
  u8g2.setCursor(0, 40);
  u8g2.print(pass);
  
  u8g2.sendBuffer();
  
  

}

void showOLED_changing_WiFi(){
  u8g2.clearBuffer();
  u8g2_prepare();
  
  u8g2.setCursor(0, 15);
  u8g2.print("Changing WiFi");
  u8g2.setCursor(0, 30);
  u8g2.print("WiFi Manager");
  u8g2.setCursor(0, 40);
  u8g2.print("Go to 192.168.4.1");

  u8g2.sendBuffer();

  

}
