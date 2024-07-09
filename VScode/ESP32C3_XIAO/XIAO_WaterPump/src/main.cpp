#include <Arduino.h>
#include "MyWiFi.h"
#include "MyOTA.h"
#include <WebSerial.h>
#include <Wire.h>

//#define ADAFRUIT


#ifdef ADAFRUIT

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#else 

#include "U8g2lib.h"

const int SDA_PIN = GPIO_NUM_6;
const int SCL_PIN = GPIO_NUM_7;

U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE);

#endif



char apName[] = "WATER_XIAO";

AsyncWebServer server(80);
MyWiFi* myWiFi = new MyWiFi(&server);
MyOTA* myOTA = new MyOTA();

void initWebSerial();
void checkI2C(int _delay);

void setup() {
  Serial.begin(115200);


#ifdef ADAFRUIT
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
#else 
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
#endif
  
  myWiFi->beginWiFi(apName);
  initWebSerial();
  myOTA->initOTA(&server, true);
  
  u8g2.setCursor(0, 16);
  u8g2.printf(apName);
  u8g2.setCursor(0, 32);
  u8g2.printf(WiFi.localIP().toString().c_str());
  u8g2.sendBuffer();
}

uint32_t count = 0;
unsigned long countTime = 0;
void loop() {
  
  if(millis() - countTime > 2000)
  {
    countTime = millis();
    Serial.printf("COUNT : %d\r\n", count++);


#ifdef ADAFRUIT
    display.clearDisplay();
    display.setTextSize(2);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.printf("%d", count);
    display.display();
#else
    // u8g2.clearBuffer(); // 전체 클리어
    // u8g2.clear();  // 위에서 아래로 천천히 지워짐

    // u8g2.drawStr(0, 16, "1234"); // 커서 위치 Y :  0 ~ 63 => 절반(32) 16 ~ 32(글자크기로 47까지 표기)
    // u8g2.sendBuffer();
    
    u8g2.setCursor((sizeof(apName)+1)*7 , 16);    // Y : 16 ~ 32
    u8g2.printf(": %5d", count);
    u8g2.sendBuffer();
    
#endif

  }

  myOTA->loopOTA();
  // myWiFi->reconnect();
  // checkI2C(1000);

}



void initWebSerial()
{
  WebSerial.onMessage([](const String& msg) { Serial.println(msg); }); 
  WebSerial.begin(&server);
  WebSerial.setBuffer(128);
  server.onNotFound([](AsyncWebServerRequest* request) { request->redirect("/webserial"); });
}

void checkI2C(int _delay){
  // delay(1000);
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(_delay); 
}