#include "OTA.h"
#include "Arduino.h"
#include "Neopixel.h"
#include "FileSystem.h"

unsigned long connectingTime = 0;
unsigned long connecting_interval = 10000;
int WiFiManager_flag = 0;
int connection_flag = 0;

extern char ssid[32];
extern char pass[32];
extern float timeZone;
extern uint8_t summerTime;


char TEMP_SSID[32] = {0,};
char TEMP_PASS[32] = {0, };

extern volatile int count0;
extern volatile int count1; 
extern portMUX_TYPE timerMux;

extern bool FORMAT_SPIFFS_IF_FAILED;  // true

void initWiFi() {
  init_SPIFFS(FORMAT_SPIFFS_IF_FAILED);
  listDir("/");
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  Serial.println("--------Saved Data--------");
  if (SPIFFS.exists("/config.txt")) loadConfig();
  else saveConfig();
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  //readFile("/config.txt");
  Serial.println("--------------------------");
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.print('.');
    //delay(1000);
    blinkNeopixel(0, 255, 0, 2);
    connecting_interval = millis();
    if(connecting_interval >= connecting_interval)
    {
      Serial.println();
      Serial.println("Not Connected -> Open WiFi Manager");
      pickOneLED(0, 255, 0, 0, 50, 50);
      WiFiManager_flag = 1;
      break;
    }
  }
  if(WiFiManager_flag == 1)
  {
    wm.resetSettings();
    bool res;
    res = wm.autoConnect("RemoteLED");
    
    if(!res) 
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
        blinkNeopixel(0, 0, 255, 2);
        SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
        PW_NAME.toCharArray(TEMP_PASS, sizeof(TEMP_PASS));

        for(int i=0; i< sizeof(ssid); i++){
          ssid[i] = TEMP_SSID[i];
          pass[i] = TEMP_PASS[i];
        }
        saveConfig();
        
        ESP.restart();
    }
  }
  else 
  {
    Serial.print("connected ->");
    Serial.println(WiFi.localIP());
//    showOLED_IP_Address();
    pickOneLED(0, 0,   0,   255, 50, 50);
    
  }
//  readWiFiEEPROM();
  Serial.println("----------Result----------");
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  Serial.println(timeZone);
  Serial.println(summerTime);
  Serial.println("--------------------------");
}

void changeWiFi(){
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  blinkNeopixel(255, 255, 0, 2);
  WiFiManager wm;
  wm.resetSettings();
  bool res;
  res = wm.autoConnect("Change_ESP_WiFi");
  if(!res) 
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
      blinkNeopixel(0, 0, 255, 2);
      SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
      PW_NAME.toCharArray(TEMP_PASS, sizeof(TEMP_PASS));

      for(int i=0; i< sizeof(ssid); i++){
        ssid[i] = TEMP_SSID[i];
        pass[i] = TEMP_PASS[i];
      }
      saveConfig();
      pickOneLED(0, 255, 255, 0, 50, 1000);
      ESP.restart();
  }
}

void reconnectWiFi(){
  if(count1 >0 ){
    portENTER_CRITICAL(&timerMux); // 카운트 시작 
    count1--;                       // 트리거 시간 감소
    portEXIT_CRITICAL(&timerMux);  // 카운트 종료
    Serial.println("reconnect working");
    if ((WiFi.status() != WL_CONNECTED)) {
      Serial.println(millis());
      Serial.println("Reconnecting to WiFi...");
      
      WiFi.disconnect();
      WiFi.reconnect();
      connection_flag = 1;
    }
    else if(connection_flag == 1)
    {
      Serial.println("reconnected");  
      connection_flag = 0;
    }
  }
}
