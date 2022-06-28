#include <esp_now.h>
#include "WiFi.h"
#include <esp_wifi.h> // esp_now.h  포함
//#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

String success;
uint8_t incomingRGB[3];

unsigned long t = 0;

#pragma pack(push, 1)
typedef struct packet_
{
    uint8_t device_led;
    uint8_t RED;  // 데이터 타입 : 1
    uint8_t GREEN;
    uint8_t BLUE;
    uint8_t brightness;
    uint8_t style;            // 데이터 : 4 
    uint8_t wait;
    uint8_t checksum;  // 체크섬 : 2
}PACKET;
#pragma pack(pop)

typedef enum {
  oneColor = 1,
  CHASE,
  RAINBOW
}STYLE_Typedef;

//STYLE_Typedef _style;

PACKET serial_data = {0, };
PACKET incomingReadings;
PACKET sample_data1 = {0x10, 255, 0, 0, 10, 1, 20, 0};
PACKET sample_data2 = {0x10, 0, 0, 255, 0, 1, 20, 0};

int neopixel_Flag = 0;
int broadcast_Flag = 0;
int compare_Flag = 0;
int ESPNOW_Flag = 0;
char ssid[32] = {0,};
int8_t channel[255] = {0, };
int8_t temp_channel = 0;
esp_now_peer_info_t peerInfo;

char compare_esp[] = "ESP";
char compare_Remote[] = "Remote";
char ssid_esp[3] = {0,};
char ssid_Remote[6] = {0,};


//constexpr char WIFI_SSID[] = "2KNG";


int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}


void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");

  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }

  broadcast_Init();

  
  while (true){
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int network = WiFi.scanNetworks();
    Serial.println("scan done");
    if (network == 0) 
    {
      Serial.println("no networks found");
    } 
    else 
    {
      Serial.print(network);
      Serial.println(" networks found");
      for (int i = 0; i < network; ++i) 
      {   // 1번 부터
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");           
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?"_":"*");

          delay(10);           
       }                 
    }     
    Serial.println("");
      
    for(int i=0; i < network; ++i)
    {
      WiFi.SSID(i).toCharArray(ssid, sizeof(ssid));   // 배열로

      for(int j=0; j < sizeof(ssid_esp); j++)
      {
        ssid_esp[j] = ssid[j];
      }

      for(int k=0; k < sizeof(ssid_Remote); k++)
      {
        ssid_Remote[k] = ssid[k];
      }
      
      if(!strcmp(compare_esp, ssid_esp))
      {
        compare_Flag = 1;

        Serial.print("compare : ");
        Serial.println(compare_esp);
        Serial.print("ssid[3] : ");
        Serial.println(ssid_esp);
        Serial.print("FLAG : ");
        
        
        channel[i] = getWiFiChannel(ssid);
        Serial.print("Channel : ");
        Serial.println(channel[i]);
        temp_channel = channel[i];
        break;
      }
      else if(!strcmp(compare_Remote, ssid_Remote))
      {
        compare_Flag = 1;

        Serial.print("compare : ");
        Serial.println(compare_Remote);
        Serial.print("ssid[6] : ");
        Serial.println(ssid_Remote);
        Serial.print("FLAG : ");
        
        
        channel[i] = getWiFiChannel(ssid);
        Serial.print("Channel : ");
        Serial.println(channel[i]);
        temp_channel = channel[i];
        break;
      }
      
    }
    if(compare_Flag == 1)
    {
      esp_wifi_set_promiscuous(true);
      esp_wifi_set_channel(temp_channel, WIFI_SECOND_CHAN_NONE);
      esp_wifi_set_promiscuous(false);
      broadcast((uint8_t *) &sample_data1, sizeof(sample_data1));
      delay(500);
        
      if(broadcast_Flag == 1){
        Serial.println("Complete Broadcast");
        break;
      }
    }
    
    // Wait a bit before scanning again
    delay(3000);
    broadcast_Flag = 0;
    compare_Flag = 0;
    
  }

   // Init ESP-NOW
  broadcast((uint8_t *) &sample_data2, sizeof(sample_data2));
  Serial.write("\r\nSetup_Done");
  
}


void loop() {
  if(Serial.available())
  {
      // packet 사이즈만큼 읽어옴
      Serial.readBytes((char*)&serial_data, sizeof(serial_data));
      serial_data.checksum += 1;
      neopixel_Flag = 1;
      Serial.println("-----------------------");      
      //Serial.write((char*)&serial_data, sizeof(serial_data));
      delay(1);
  }


  if( neopixel_Flag == 1 ){
    neopixel_Flag = 0;  
    broadcast((uint8_t *) &serial_data, sizeof(serial_data));
  }
  
}

void broadcast_Init(){
  // Register peer
  esp_now_peer_info_t peerInfo = {};
  //memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6); // 6-> address length
  peerInfo.encrypt = false; // 암호화 ID/PW

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESPNOW_Flag = 0;
    Serial.println("Failed to add peer");
    return;
  }else{
    Serial.println("Succeed to add peer"); 
    ESPNOW_Flag = 1;
    esp_now_register_send_cb(OnDataSent);
//    esp_now_register_recv_cb(OnDataRecv);
  }
}


void scan_WiFi(){
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int network = WiFi.scanNetworks();
  Serial.println("scan done");
  if (network == 0) 
  {
      Serial.println("no networks found");
  } 
  else 
  {
      Serial.print(network);
      Serial.println(" networks found");
      for (int i = 0; i < network; ++i) 
      {   // 1번 부터
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");           
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?"_":"*");

          delay(10);           
       }                 
   }     
   Serial.println("");
}




// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { 
  char macStr[18];
  formatMacAddress(mac_addr, macStr, 18);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr);
  
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
    broadcast_Flag = 1;
  }
  else{
    success = "Delivery Fail :(";
    broadcast_Flag = 0;
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}


void broadcast(const uint8_t * broadcastData, int dataSize)
{
  // this will broadcast a message to everyone in range
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)broadcastData, dataSize);

}
