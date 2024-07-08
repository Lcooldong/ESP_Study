#ifndef __MYWIFI_H__
#define __MYWIFI_H__

#include <ESPAsyncWiFiManager.h>
#include "MyLittleFS.h"
#include <WiFi.h>

class MyWiFi
{
private:
    
    DNSServer dns;
    AsyncWiFiManager* wifiManager;
    
    unsigned long previousMillis = 0;
    unsigned long interval = 30000;
    const unsigned int WIFI_CONNECTION_INTERVAL = 10000;

public:
    MyWiFi(AsyncWebServer* _server);

    ~MyWiFi();

    void beginWiFi(char* _APNAME);
    void initWiFi();
    void beginWiFiManager(char* _APNAME);
    void reconnect();

    MyLittleFS* mySPIFFS = new MyLittleFS();
};




#endif