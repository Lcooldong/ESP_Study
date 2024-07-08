#include <MyWiFi.h>
#include <Arduino.h>


MyWiFi::MyWiFi(AsyncWebServer* _server)
{

  wifiManager = new AsyncWiFiManager(_server, &dns);
}

MyWiFi::~MyWiFi()
{
}


void MyWiFi::beginWiFi(char* _APNAME)
{
  mySPIFFS->InitLitteFS();
  delay(100);

  mySPIFFS->listDir(LittleFS, "/", 0);
  Serial.println("Connecting to WiFi...");

  if(mySPIFFS->loadConfig(LittleFS))
  {
    initWiFi();
  }
  else
  {
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }
  delay(500);
  beginWiFiManager(_APNAME);
}

void MyWiFi::initWiFi()
{
    Serial.println(mySPIFFS->ssid);
    Serial.println(mySPIFFS->pass);

    WiFi.mode(WIFI_STA); // 
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.println("\r\n--While Connecting--");
    }
    Serial.printf("IP : %s\r\n", WiFi.localIP().toString().c_str());
    Serial.println("Connect to Flash Memory");
}

void MyWiFi::beginWiFiManager(char* _APNAME)
{
  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      if (millis() - connectionLastTime > WIFI_CONNECTION_INTERVAL)
      {
        Serial.println("Start WiFiManager => 192.168.4.1");

        wifiManager->resetSettings();
        bool wmRes = wifiManager->autoConnect(_APNAME);
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.printf("\nSuccess\n");
          mySPIFFS->saveConfig(LittleFS, wifiManager->getConfiguredSTASSID(), wifiManager->getConfiguredSTAPassword());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }

  }
}

void MyWiFi::reconnect()
{
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
}
