#include <Arduino.h>
//#include "M5AtomS3.h"

#include "WiFi.h"
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include "ESPAsyncWiFiManager.h"

#include "ImageViewer.hpp"
#include "MyLittleFS.h"
#include "img_res.c"

MyLittleFS* myLittleFS = new MyLittleFS();
AsyncWebServer server(80);
DNSServer dns;

ImageViewer viewer;
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "mqtt.m5stack.com";

bool btnToggle = false;

void forever(void) {
    while (true) {
        delay(1);
    }    
}

void setup() {
  // M5.begin(true, true, true, true);
   
 // myLittleFS->InitLitteFS();
  USBSerial.begin(115200);
  USBSerial.flush();
  delay(1200);
  // SPIFFS 적용됨
  if (!viewer.begin()) {
        forever();
  }
  
  myLittleFS->listDir(SPIFFS, "/", 0);
  USBSerial.printf("%s\n", "hello");

  // AsyncWiFiManager wifiManager(&server,&dns);
  // wifiManager.autoConnect("AutoConnectAP");
  // Serial.println("connected...yeey :)");

  // M5.IMU.begin();
  // USBSerial.printf("0x%02x\n", M5.IMU.whoAmI());
}

float ax, ay, az, gx, gy, gz, t;
uint8_t num[3] = {0x00, 0x00, 0x00};



void loop() {
  viewer.update();
    delay(100);
  // M5.update();
  // //M5.Lcd.drawCircle(64, 64, 4, 0xFFFFFF);
  // M5.Lcd.drawBitmap(64, 64, 10, 10,  num);
  // num[0] += 1;
  // num[1] += 1;
  // USBSerial.printf( "%d\n", num[0]);
  // //num[1] += 1;
  // delay(100);
  // // 한번 눌렀을 때(020), 꾹 눌렀을 때(02111)
  // int btn_state = M5.Btn.pressedFor(1000) ? 1 : M5.Btn.wasPressed() ? 2 : 0;
  // USBSerial.print(btn_state);
  // delay(1000);
  

  //M5.

  // if(M5.Btn.wasReleased())
  // {
  //   if (!btnToggle)
  //   {
  //     M5.Lcd.print("PC ON\n");
  //     //USBSerial.print('A');
  //   }
  //   else
  //   {
  //     M5.Lcd.print("PC OFF\n");
  //     //USBSerial.print('B');
  //   }
  //   btnToggle = !btnToggle;
  //   USBSerial.print(btnToggle);
  //   delay(500);
  // }
  // else if (M5.Btn.pressedFor(1000)) // PC 가 켜져있을 때 조건 넣기
  // {
  //   USBSerial.print("PRESSED");
  //   M5.dis.drawpix(0xff0000);
  //   M5.dis.show();
  //   delay(500);
  //   M5.dis.drawpix(0x00ff00);
  //   M5.dis.show();
  //   delay(500);
  //   M5.dis.drawpix(0x0000ff);
  //   M5.dis.show();
  //   delay(500);
  // }

  // M5.Lcd.setCursor(0, 40);
  // M5.Lcd.clear();
  // M5.IMU.getAccel(&ax, &ay, &az);
  // M5.IMU.getGyro(&gx, &gy, &gz);
  // M5.IMU.getTemp(&t);

  // USBSerial.printf("%f | %f | %f | %f | %f | %f | %f\n", ax, ay, az, gx, gy, gz, t);

  // M5.Lcd.printf("IMU:\r\n");
  // M5.Lcd.printf("%0.2f %0.2f %0.2f\r\n", ax, ay, az);
  // M5.Lcd.printf("%0.2f %0.2f %0.2f\r\n", gx, gy, gz);
  // delay(500);
}