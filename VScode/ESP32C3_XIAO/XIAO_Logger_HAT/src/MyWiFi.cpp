#include "MyWiFi.h"
#include "MyLittleFS.h"
#include <WiFiManager.h>


MyLittleFS* mySPIFFS = new MyLittleFS();
WiFiManager wifiManager;

void beginWiFi(String apName)
{
  mySPIFFS->InitLitteFS();
  delay(100);

  mySPIFFS->listDir(LittleFS, "/", 0);
  Serial.println("Connecting to WiFi...");

  if(mySPIFFS->loadConfig(LittleFS))
  {
    init_WiFi();
  }
  else
  {
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }
  delay(500);

  beginWiFiManager(apName);

}


void init_WiFi()
{
    Serial.println(mySPIFFS->ssid);
    Serial.println(mySPIFFS->pass);

    WiFi.mode(WIFI_STA); // 
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    //delay(3000);
    Serial.printf("IP : %s\r\n", WiFi.localIP().toString().c_str());
    Serial.println("Connect to Flash Memory");

}


void beginWiFiManager(String apName)
{
  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      if (millis() - connectionLastTime > 10000)
      {
        Serial.println("Start WiFiManager => 192.168.4.1");
        wifiManager.resetSettings();
        bool wmRes = wifiManager.autoConnect(apName.c_str());
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.printf("\nSuccess\n");
          Serial.printf("%s  | %s \r\n", wifiManager.getWiFiSSID().c_str(), wifiManager.getWiFiPass().c_str() );
          mySPIFFS->saveConfig(LittleFS, wifiManager.getWiFiSSID().c_str(), wifiManager.getWiFiPass().c_str());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }

  }
}

