#include <esp_now.h>
#include <WiFi.h>

uint8_t wemos_d1mini[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
uint8_t esp32_38pin[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};

String success;
uint8_t incomingRGB[3];
unsigned long t = 0;

typedef struct _struct_message {
    uint8_t RGB[3];
} struct_message;

struct_message incomingReadings;
struct_message myState;

//void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
//void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

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
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, wemos_d1mini, 6); // 6-> address length
  peerInfo.channel = 0;
  peerInfo.encrypt = false; // 암호화 ID/PW

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }else{
    Serial.println("Succeed to add peer");  
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
}

void loop() {

  
  if(millis() - t > 2000){
    t = millis();
    //2초마다 한번씩 하겠다!
    //esp_now_send(wemos_d1mini, (uint8_t *) &data, sizeof(data));
  }


}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  for(int i = 0; i<sizeof(incomingRGB); i++)
  {
      incomingRGB[i] = incomingReadings.RGB[i];
  }

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(wemos_d1mini, (uint8_t *) &incomingReadings, sizeof(incomingReadings));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  Serial.print("RECEIVE DATA:");
  //Serial.println(incomingState);
}
