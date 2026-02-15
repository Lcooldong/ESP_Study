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
    uint8_t led_number; // 시작 사인 : 1
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

STYLE_Typedef _style;

PACKET serial_data = {0, };
PACKET incomingReadings;

int neopixel_Flag = 0;

esp_now_peer_info_t peerInfo;

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
  esp_now_peer_info_t peerInfo = {};
  //memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, esp_module_0, 6); // 6-> address length
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


  serial_data.led_number = 1;
  serial_data.RED = 255;
  serial_data.GREEN = 0;
  serial_data.BLUE = 123;
  serial_data.brightness = 50;
  serial_data.style = 1;
  serial_data.wait = 50;
  serial_data.checksum = 0;

  
}

void loop() {
  if(Serial.available())
  {
    char ch = Serial.read();
    if(ch == 's')
    {
      serial_data.led_number = 1;
      serial_data.RED = 255;
      serial_data.GREEN = 0;
      serial_data.BLUE = 123;
      serial_data.brightness = 50;
      serial_data.style = 1;
      serial_data.wait = 50;
      serial_data.checksum = 0;
      esp_err_t result = esp_now_send(esp_module_0, (uint8_t *) &serial_data, sizeof(serial_data));

      if (result == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
    } 
    delay(100);
  }
  

  


}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status: ");
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


  Serial.print("Bytes received: ");
  Serial.println(len);

  incomingRGB[0] = incomingReadings.RED;
  incomingRGB[1] = incomingReadings.GREEN;
  incomingRGB[2] = incomingReadings.BLUE;
  

  Serial.print("수신 -> Red : ");
  Serial.println(incomingRGB[0]);
  Serial.print("Green : ");
  Serial.println(incomingRGB[1]);
  Serial.print("Blue : ");
  Serial.println(incomingRGB[2]);

  esp_err_t result = esp_now_send(esp_module_0, (uint8_t *) &incomingReadings, sizeof(incomingReadings));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

}
