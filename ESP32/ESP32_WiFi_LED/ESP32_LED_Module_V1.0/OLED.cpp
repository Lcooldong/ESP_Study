#include "OLED.h"
#include <WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initOLED(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  // Display static text
  display.println("Hello, world!");
  display.display(); 
}


void showOLED_IP_Address(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 15);
  display.println(WiFi.localIP());
}


void showOLED_WiFi(char* ssid, char* pass){
  display.setCursor(0, 30);
  display.println(ssid);
  display.setCursor(0, 40);
  display.println(pass);
  display.display();
}

void showOLED_changing_WiFi(){
  display.clearDisplay();
  display.setCursor(0, 15);
  display.println("Changing WiFi");
  display.setCursor(0, 30);
  display.println("WiFi Manager");
  display.setCursor(0, 40);
  display.println("Go to 192.168.4.1");
  display.display();
}
