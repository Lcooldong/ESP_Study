#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <ESPAsyncWiFiManager.h>
#include "MyLittleFS.h"

#define WIFI_CONNECTION_INTERVAL 10000

uint64_t lastTime = 0;
AsyncWebServer server(80);
DNSServer dns;
MyLittleFS* mySPIFFS = new MyLittleFS();

void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
  // if (d == "ON"){
  // digitalWrite(LED, HIGH);
  // }
  // if (d=="OFF"){
  //   digitalWrite(LED, LOW);
  // }
}

void setup() {
  Serial.begin(115200);

  AsyncWiFiManager wifiManager(&server,&dns);

  mySPIFFS->InitLitteFS();

  if(mySPIFFS->loadConfig(LittleFS))
  {
    Serial.println(mySPIFFS->ssid);
    Serial.println(mySPIFFS->pass);


    WiFi.mode(WIFI_STA);
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    Serial.println("Connect to Flash Memory");

  }
  else
  {
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }

  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      if (millis() - connectionLastTime > WIFI_CONNECTION_INTERVAL)
      {
        Serial.println("Start WiFiManager => 192.168.4.1");
        wifiManager.resetSettings();
        bool wmRes = wifiManager.autoConnect("OTA_TEST");
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.printf("\nSuccess\n");
         
          mySPIFFS->saveConfig(LittleFS, wifiManager.getConfiguredSTASSID(), wifiManager.getConfiguredSTAPassword());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }

  }

  


  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();

}

int cnt = 0;

void loop() {
  ArduinoOTA.handle();
  if(millis() - lastTime > 2000)
  {
    lastTime = millis();
    cnt++;
    WebSerial.printf("%d : Hello!\r\n", cnt);
    Serial.printf("%d : Hello!\r\n", cnt);
  }
}

