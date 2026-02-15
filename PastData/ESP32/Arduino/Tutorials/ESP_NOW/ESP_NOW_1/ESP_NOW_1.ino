//up to 250 bytes payload

#include <esp_now.h>
#include <WiFi.h>
#define LED_PIN 15
//#include <Wire.h>

//uint8_t broadcastAddress[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};  // 38pin Mac Address
uint8_t broadcastAddress[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
String success;
bool incomingState;

typedef struct struct_message {
    bool ledState;
} struct_message;

struct_message incomingReadings;
struct_message myState;

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
  incomingState = incomingReadings.ledState;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(LED_PIN, OUTPUT);

    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);


  myState.ledState = false;
}

void loop() {

  myState.ledState = !myState.ledState;
  Serial.print("myLEDSTATE : ");
  Serial.println(myState.ledState);
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myState, sizeof(myState));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  Serial.print("RECEIVE DATA:");
  Serial.println(incomingState);
  digitalWrite(LED_PIN, incomingState);
  delay(2000);
}
