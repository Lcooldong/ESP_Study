#include <esp_now.h>
#include <WiFi.h>

uint8_t wemos_d1mini[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
uint8_t esp32_38pin[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};
uint8_t wemos_lite[] = {0x78, 0xE3, 0x6D, 0x19, 0x2F, 0x44};
uint8_t esp_module_0[] = {0x58, 0xBF, 0x25, 0x17, 0x6D, 0x28};

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

int neopixel_Flag = 0;

esp_now_peer_info_t peerInfo;



void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  //memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, wemos_lite, 6); // 6-> address length
  peerInfo.channel = 1;
  peerInfo.encrypt = false; // 암호화 ID/PW

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }else{
    Serial.println("Succeed to add peer"); 
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
  }
}



void loop() {
  if(Serial.available())
  {
      // packet 사이즈만큼 읽어옴
      Serial.readBytes((char*)&serial_data, sizeof(serial_data));
      serial_data.checksum += 1;
      neopixel_Flag = 1;
      Serial.println("-----------------------");
      broadcast((uint8_t *) &serial_data, sizeof(serial_data));
      //Serial.write((char*)&serial_data, sizeof(serial_data));
      delay(50);
  }


  if( neopixel_Flag == 1 ){
    
    neopixel_Flag = 0;  

  }
  

  


}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//  char macStr[18];
//  formatMacAddress(mac_addr, macStr, 18);
//  Serial.print("Last Packet Sent to: ");
//  Serial.println(macStr);
//  
//  Serial.print("Last Packet Send Status: ");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
//  if (status ==0){
//    success = "Delivery Success :)";
//  }
//  else{
//    success = "Delivery Fail :(";
//  }
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
