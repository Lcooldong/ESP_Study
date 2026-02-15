#include <WiFi.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#define EEPROM_SIZE 128

// Replace with your network credentials (STATION)
//const char* ssid = "IT";
//const char* password = "@Polytech";
const char* ssid = "LDH";
const char* password = "ehdgml43";

unsigned long connectingTime = 0;
unsigned long connecting_interval = 10000;
int WiFiManger_flag = 0;

char TEMP_SSID[32] = {0,};
char TEMP_PW[32] = {0, };

char EEPROM_SSID[32];
char EEPROM_PW[32];

unsigned long previousMillis = 0;
unsigned long interval = 30000;
int connection_flag = 0;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  Serial.println("--------Saved Data--------");
  readWiFiEEPROM();
  Serial.println("--------------------------");
  WiFi.begin(EEPROM_SSID, EEPROM_PW);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
    connecting_interval = millis();
    if(connecting_interval >= interval)
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

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  

}

void loop() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.println(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    connection_flag = 1;
    previousMillis = currentMillis;
  }
  else if(connection_flag == 1)
  {
    Serial.println("reconnected");  
    connection_flag = 0;
  }

  
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
    Serial.print("EEPROM SSID : ");
    Serial.println(EEPROM_PW);  
}
