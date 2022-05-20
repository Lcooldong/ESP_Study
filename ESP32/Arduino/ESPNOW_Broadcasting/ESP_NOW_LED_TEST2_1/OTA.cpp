#include "OTA.h"
#include "Arduino.h"
#include "FileSystem.h"
#include "Neopixel.h"
#include "OLED.h"

unsigned long connectingTime = 0;
unsigned long connecting_interval = 10000;
int WiFiManager_flag = 0;
int connection_flag = 0;
extern int neopixel_Flag;

extern char ssid[32];
extern char pass[32];
extern float timeZone;
extern uint8_t summerTime;


char TEMP_SSID[32] = {0,};
char TEMP_PASS[32] = {0, };

extern volatile int count0;
extern volatile int count1; 
extern portMUX_TYPE timerMux;
int blinkToggle = 0;
extern Adafruit_NeoPixel strip;

extern bool FORMAT_SPIFFS_IF_FAILED;  // true

void initWiFi() {
  init_SPIFFS(FORMAT_SPIFFS_IF_FAILED);
  listDir("/");
  Wire.begin(23, 19); // SDA, SCL
  initOLED();
  WiFi.mode(WIFI_AP_STA);
  WiFiManager wm;
  Serial.println("--------Saved Data--------");
  if (SPIFFS.exists("/config.txt")) loadConfig();
  else saveConfig();
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  showOLED_WiFi(ssid, pass);
  //readFile("/config.txt");
  Serial.println("--------------------------");
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  pickOneLED(0, strip.Color(255, 0, 0), 5, 50);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    
    Serial.print('.');
    delay(1000);   
    connecting_interval = millis();
    if(connecting_interval >= connecting_interval)
    {
      Serial.println();
      Serial.println("Not Connected -> Open WiFi Manager");
      
      WiFiManager_flag = 1;
      break;
    }
  }
  if(WiFiManager_flag == 1)
  {
    blinkNeopixel(strip.Color(255, 0, 0), 5, 500);
    pickOneLED(0, strip.Color(255, 0, 0), 5, 50);
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
        blinkNeopixel(strip.Color(0, 255, 0), 5, 400);
        pickOneLED(0, strip.Color(255, 0, 0), 5, 50);
        String SSID_NAME = WiFiManager().getWiFiSSID();
        String PW_NAME = WiFiManager().getWiFiPass();
        Serial.print("SSID : ");
        Serial.println(SSID_NAME);
        
        Serial.print("PASSWORD : ");
        Serial.println(PW_NAME);
        
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        
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
    showOLED_IP_Address();
    pickOneLED(0, strip.Color(0, 255, 0), 1, 50);
    
  }
//  readWiFiEEPROM();
  Serial.println("----------Result----------");
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  showOLED_WiFi(ssid, pass);
//  Serial.println(timeZone);
//  Serial.println(summerTime);
  Serial.println("--------------------------");
}

void changeWiFi(){
  blinkNeopixel(strip.Color(255, 0, 0), 5, 500);
  pickOneLED(0, strip.Color(255, 0, 0), 50, 50);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFiManager wm;
  wm.resetSettings();
  showOLED_changing_WiFi();
  bool res;
  res = wm.autoConnect("Change_ESP_WiFi");
  if(!res) 
  {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else 
  {
      blinkNeopixel(strip.Color(0, 255, 0), 5, 500);
      String SSID_NAME = WiFiManager().getWiFiSSID();
      String PW_NAME = WiFiManager().getWiFiPass();
      Serial.print("SSID : ");
      Serial.println(SSID_NAME);
       
      Serial.print("PASSWORD : ");
      Serial.println(PW_NAME);
      
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
      SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
      PW_NAME.toCharArray(TEMP_PASS, sizeof(TEMP_PASS));

      for(int i=0; i< sizeof(ssid); i++){
        ssid[i] = TEMP_SSID[i];
        pass[i] = TEMP_PASS[i];
      }
      saveConfig();
      showOLED_IP_Address();
      showOLED_WiFi(ssid, pass);
      pickOneLED(0, strip.Color(0, 255, 0), 50, 1000);
      pickOneLED(0, strip.Color(0, 0, 0), 0, 10);
//      ESP.restart();
  }
}

void reconnectWiFi(){
  if(count0 >0 ){
    portENTER_CRITICAL(&timerMux); // 카운트 시작 
    count0--;                       // 트리거 시간 감소
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
      blinkNeopixel(strip.Color(0, 0, 255), 4, 500);
      connection_flag = 0;
    }
  }
}

void blinkTimer(uint32_t color){
  if(count1 > 0){
    portENTER_CRITICAL(&timerMux); // 카운트 시작 
    count1--;                       // 트리거 시간 감소
    portEXIT_CRITICAL(&timerMux);  // 카운트 종료
    Serial.println("Blink Timer");
    if(neopixel_Flag == 1){
      neopixel_Flag = 0;
      Serial.print("Blink Status : ");
      Serial.println(blinkToggle);
      blinkToggle = !blinkToggle;
      pickOneLED(0, color, 50*blinkToggle, 50);       
    }     
  }  
}
