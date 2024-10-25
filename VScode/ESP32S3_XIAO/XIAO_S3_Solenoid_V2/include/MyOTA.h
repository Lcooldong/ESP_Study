#ifndef __MYOTA_H__
#define __MYOTA_H__

#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ElegantOTA.h>
#include <WebSerial.h>

#define WIFI_CONNECTION_INTERVAL 10000
#define KOR_GMT_9 32400

#define OTA_DEBUG

class MyOTA
{
private:

    const char* deviceName = "Seeed S3";

    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = KOR_GMT_9;
    const int daylightOffset_sec = 0;

    AsyncWebServer* _server = new AsyncWebServer(80);
    DNSServer dns;
    WiFiClient espClient;
    AsyncWiFiManager wifiManager = AsyncWiFiManager(_server, &dns);
    


    uint64_t ota_progress_millis = 0;
    int reconnectCount = 0;

    void onOTAStart();
    void onOTAProgress(size_t current, size_t final);
    void onOTAEnd(bool success);

    void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
    void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);  

public:
    MyOTA();
    ~MyOTA();


    void initOTA();
    void loop();

    void initWiFi();
    void beginWiFiManager();


protected:


  
    // class MyWiFi
    // {
    //     public:
            
    // };

};




#endif