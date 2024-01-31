#include "main.h"

unsigned long ota_progress_millis = 0;

AsyncWebServer server(80);
DNSServer dns;
MyLittleFS* mySPIFFS = new MyLittleFS();


void setup(void) {
  Serial.begin(115200);

  setUpWiFi();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "For Update -> /update \r\nFor Debug  -> /webserial");
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);

  arduinoOTAProgress();

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  ArduinoOTA.handle();
  ElegantOTA.loop();

}


void setUpWiFi()
{
  AsyncWiFiManager wifiManager(&server,&dns);

  mySPIFFS->InitLitteFS();

  if(mySPIFFS->loadConfig(LittleFS))
  {
//    Serial.println(mySPIFFS->ssid);
//    Serial.println(mySPIFFS->pass);
#ifdef FIXED_IP
    IPAddress ip (192, 168, 1, 48);
    IPAddress gateway (192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);
#endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    Serial.print("Connect to Flash Memory!");
  }
  else
  {
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }

  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
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
  Serial.print("\nIP : ");
  Serial.println(WiFi.localIP());

}

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

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


void arduinoOTAProgress()
{
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
}