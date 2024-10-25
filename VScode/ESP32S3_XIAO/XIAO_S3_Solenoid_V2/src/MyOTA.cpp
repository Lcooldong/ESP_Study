#include "MyOTA.h"

// #include <WebSerial.h>
#include "MyLittleFS.h"
#include <time.h>

extern MyLittleFS* myFS;
MyOTA::MyOTA() : ota_progress_millis(0), reconnectCount(0) {}


MyOTA::~MyOTA()
{
}

void MyOTA::onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void MyOTA::onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - this->ota_progress_millis > 1000) {
    this->ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void MyOTA::onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}



void MyOTA::initOTA()
{
    
  myFS->InitLitteFS();

  delay(100);

  myFS->listDir(LittleFS, "/", 0);
  Serial.println("Connecting to WiFi...");

  if(myFS->loadConfig(LittleFS))
  {
    initWiFi();
  }
  else
  {
    //const char* ssid        = "..........";
    //const char* password    = "..........";
    //WiFi.begin(ssid, password);
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }
  delay(500);
  beginWiFiManager();
  

  configTime(this->gmtOffset_sec, this->daylightOffset_sec, this->ntpServer);
  // printLocalTime();

  WebSerial.onMessage([](const String& msg) { Serial.println(msg); }); 
  WebSerial.begin(this->_server);
  WebSerial.setBuffer(128);
  _server->onNotFound([](AsyncWebServerRequest* request) { request->redirect("/webserial"); });


  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is ElegantOTA AsyncDemo.");
  });

  ElegantOTA.begin(_server);    // Start ElegantOTA
  // ElegantOTA callbacks
//   ElegantOTA.onStart(onOTAStart);
//   ElegantOTA.onProgress(onOTAProgress);
//   ElegantOTA.onEnd(onOTAEnd);
  ElegantOTA.onStart([this]() { this->onOTAStart(); });
  ElegantOTA.onProgress([this](size_t current, size_t final) { this->onOTAProgress(current, final); });
  ElegantOTA.onEnd([this](bool success) { this->onOTAEnd(success); });

  _server->begin();

}

void MyOTA::loop()
{
   ElegantOTA.loop();
}



void MyOTA::initWiFi()
{
  Serial.println(myFS->ssid);
  Serial.println(myFS->pass);

// static
//   WiFi.onEvent(MyOTA::WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
//   WiFi.onEvent(MyOTA::WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
//   WiFi.onEvent(MyOTA::WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) { this->WiFiStationConnected(event, info); }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) { this->WiFiGotIP(event, info); }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) { this->WiFiStationDisconnected(event, info); }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.mode(WIFI_STA); // 
  WiFi.begin(myFS->ssid, myFS->pass);
    //delay(3000);
  Serial.printf("IP : %s\r\n", WiFi.localIP().toString().c_str());
  Serial.println("Connect to Flash Memory");
  
}

void MyOTA::beginWiFiManager()
{
  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      if (millis() - connectionLastTime > WIFI_CONNECTION_INTERVAL)
      {
        Serial.println("Start WiFiManager => 192.168.4.1");

        wifiManager.resetSettings();
        bool wmRes = wifiManager.autoConnect(deviceName);
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.printf("\nSuccess\n");
          myFS->saveConfig(LittleFS, wifiManager.getConfiguredSTASSID(), wifiManager.getConfiguredSTAPassword());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }
  }
}

void MyOTA::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
}

void MyOTA::WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void MyOTA::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.printf("Trying to Reconnect %d\r\n", reconnectCount);
  WiFi.begin(myFS->ssid, myFS->pass);

  this->reconnectCount++;
  if(this->reconnectCount > 10)
  {
    this->reconnectCount = 0;
    // beginWiFiManager();
    // wifiManager.resetSettings();
    ESP.restart();
  }

}



