#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <TelnetStream.h>
#include <EEPROM.h>
#include <WiFiManager.h>

#define EEPROM_SIZE 128

void writeWiFiEEPROM();
void readWiFiEEPROM();
void initWiFi();


unsigned long connectingTime = 0;
unsigned long connecting_interval = 10000;
int WiFiManger_flag = 0;

char TEMP_SSID[32] = {0,};
char TEMP_PW[32] = {0, };

char EEPROM_SSID[32];
char EEPROM_PW[32];



#if defined(ESP32_RTOS) && defined(ESP32)
void ota_handle( void * parameter ) {
  for (;;) {
    ArduinoOTA.handle();
    delay(3500);
  }
}
#endif

void setupOTA(const char* nameprefix) {
  // Configure the hostname
  uint16_t maxlen = strlen(nameprefix) + 7;
  char *fullhostname = new char[maxlen];
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(fullhostname, maxlen, "%s-%02x%02x%02x", nameprefix, mac[3], mac[4], mac[5]);
  ArduinoOTA.setHostname(fullhostname);
  delete[] fullhostname;

  // Configure and start the WiFi station
  initWiFi();

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232); // Use 8266 port if you are working in Sloeber IDE, it is fixed there and not adjustable


  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
	//NOTE: make .detach() here for all functions called by Ticker.h library - not to interrupt transfer process in any way.

    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("\nAuth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("\nBegin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("\nConnect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("\nReceive Failed");
    else if (error == OTA_END_ERROR) Serial.println("\nEnd Failed");
  });

  ArduinoOTA.begin();
  TelnetStream.begin();

  Serial.println("OTA Initialized");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#if defined(ESP32_RTOS) && defined(ESP32)
  xTaskCreate(
    ota_handle,          /* Task function. */
    "OTA_HANDLE",        /* String with name of task. */
    10000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL);            /* Task handle. */
#endif
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  Serial.println("--------Saved Data--------");
  readWiFiEEPROM();
  Serial.println("--------------------------");
  WiFi.begin(EEPROM_SSID, EEPROM_PW);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
    connecting_interval = millis();
    if(connecting_interval >= connecting_interval)
    {
      Serial.println();
      Serial.println("Not Connected -> Open WiFi Manager");
      WiFiManger_flag = 1;
      break;
    }
  }
  if(WiFiManger_flag == 1)
  {
    wm.resetSettings();
    bool result;
    result = wm.autoConnect("RemoteLED");
    
    if(!result) 
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else 
    {
        String SSID_NAME = WiFiManager().getWiFiSSID();
        String PW_NAME = WiFiManager().getWiFiPass();
        Serial.print("SSID : ");
        Serial.println(SSID_NAME);
        
        Serial.print("PASSWORD : ");
        Serial.println(PW_NAME);
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
        PW_NAME.toCharArray(TEMP_PW, sizeof(TEMP_PW));
        writeWiFiEEPROM();
    }
  }
  else 
  {
    Serial.println();
    Serial.print("connected ->");
    Serial.println(WiFi.localIP());
    
    //strcpy(TEMP_SSID, ssid);    char* -> char[]
    //strcpy(TEMP_PW, password);
    
    //writeWiFiEEPROM();
    
  }
  readWiFiEEPROM();
}


void writeWiFiEEPROM(){
    for(int i=0; i < sizeof(EEPROM_SSID); i++)
    {
      EEPROM.write(i, 0);
      EEPROM.write(i + sizeof(EEPROM_SSID), 0);
      EEPROM.write(i, TEMP_SSID[i]);
      EEPROM.write(i + sizeof(EEPROM_SSID), TEMP_PW[i]);
   }
    EEPROM.commit();
}

void readWiFiEEPROM(){
    for(int j=0; j< sizeof(EEPROM_SSID); j++)
    {
      EEPROM_SSID[j] = EEPROM.read(j);
      EEPROM_PW[j] = EEPROM.read(j + sizeof(EEPROM_SSID));
    }
    Serial.print("EEPROM SSID : ");
    Serial.println(EEPROM_SSID);
    TelnetStream.print("EEPROM SSID : ");
    TelnetStream.println(EEPROM_SSID);
    Serial.print("EEPROM SSID : ");
    Serial.println(EEPROM_PW);
    TelnetStream.print("EEPROM SSID : ");
    TelnetStream.println(EEPROM_PW);
}
