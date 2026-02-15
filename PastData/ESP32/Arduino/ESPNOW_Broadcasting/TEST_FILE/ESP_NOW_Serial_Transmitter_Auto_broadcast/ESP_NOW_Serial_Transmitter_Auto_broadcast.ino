#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // esp_now.h  포함
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

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
PACKET sample_data2 = {0x10, 0, 0, 255, 10, 1, 20, 0};

int neopixel_Flag = 0;
int broadcast_Flag = 0;
int ESPNOW_Flag = 0;
int newtwork = 0;
char ssid[32] = {0,};
int32_t channel = 0;
esp_now_peer_info_t peerInfo;



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

  // Init ESP-NOW
  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }

  do{
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    newtwork = WiFi.scanNetworks();
    Serial.println("scan done");
    if (newtwork == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(newtwork);
        Serial.println(" networks found");
        for (int i = 0; i < newtwork; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");           
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?"_":"*");
            
            WiFi.SSID(i).toCharArray(ssid, sizeof(ssid));
            channel = getWiFiChannel(ssid);
            Serial.print("Channel : ");
            Serial.println(channel);
            delay(10);
            
       
//            WiFi.printDiag(Serial); // Uncomment to verify channel number before

            esp_wifi_set_promiscuous(true);
            esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            esp_wifi_set_promiscuous(false);
            
//            WiFi.printDiag(Serial); // Uncomment to verify channel change after
            broadcast_Init();          
            broadcast((uint8_t *) &sample_data1, sizeof(sample_data1));
            delay(500);
//            broadcast((uint8_t *) &sample_data2, sizeof(sample_data2));
            if(broadcast_Flag == 1){
              Serial.println("Complete Broadcast");
              break;
            }
        }
    }
    Serial.println("");
    if((ESPNOW_Flag == 1) && (broadcast_Flag == 1)){
      Serial.print("Broadcast Target Found : ");
      Serial.println(ssid);
      Serial.print("Channel connection : ");
      Serial.println(channel);
      
      break;    
    }
    // Wait a bit before scanning again
    delay(2000);
    broadcast_Flag = 0;
    
  }while(newtwork > 0);
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



// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//    if(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"){
//       broadcast_Flag = 1;
//    }else{
//       broadcast_Flag = 0;
//    }
    
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
//  Serial.print("Bytes received: ");
//  Serial.println(len);
//
//
//  char macStr[18];
//  formatMacAddress(mac, macStr, 18);
//  // debug log the message to the serial port
//  Serial.printf("Received message from: %s\n", macStr);
//  incomingRGB[0] = incomingReadings.RED;
//  incomingRGB[1] = incomingReadings.GREEN;
//  incomingRGB[2] = incomingReadings.BLUE;
//  
//
//  Serial.print("수신 -> Red : ");
//  Serial.println(incomingRGB[0]);
//  Serial.print("Green : ");
//  Serial.println(incomingRGB[1]);
//  Serial.print("Blue : ");
//  Serial.println(incomingRGB[2]);

//  esp_err_t result = esp_now_send(wemos_lite, (uint8_t *) &incomingReadings, sizeof(incomingReadings));
//
//  if (result == ESP_OK) {
//    Serial.println("Sent with success");
//  }
//  else {
//    Serial.println("Error sending the data");
//  }
  
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
  // and this will send a message to a specific device
  /*uint8_t peerAddress[] = {0x3C, 0x71, 0xBF, 0x47, 0xA5, 0xC0};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, peerAddress, 6);
  if (!esp_now_is_peer_exist(peerAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());*/
//  if (result == ESP_OK)
//  {
//    Serial.println("Broadcast message success");
//  }
//  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
//  {
//    Serial.println("ESPNOW not Init.");
//  }
//  else if (result == ESP_ERR_ESPNOW_ARG)
//  {
//    Serial.println("Invalid Argument");
//  }
//  else if (result == ESP_ERR_ESPNOW_INTERNAL)
//  {
//    Serial.println("Internal Error");
//  }
//  else if (result == ESP_ERR_ESPNOW_NO_MEM)
//  {
//    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
//  }
//  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
//  {
//    Serial.println("Peer not found.");
//  }
//  else
//  {
//    Serial.println("Unknown error");
//  }
}
