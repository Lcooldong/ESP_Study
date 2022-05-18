#include "OTA.h"
#include <HardwareSerial.h>
#include <esp_now.h>
#define EEPROM_SIZE 128

uint8_t wemos_d1mini[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
uint8_t esp32_38pin[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};
uint8_t wemos_lite[] = {0x78, 0xE3, 0x6D, 0x19, 0x2F, 0x44};

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


HardwareSerial mySerial(2);
PACKET _data = {0, };
PACKET incomingReadings;

STYLE_Typedef _style;

char packet_buffer[256];

int cnt = 0;
int neopixel_Flag = 0;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);



void setup() {
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 26, 27);
  mySerial.println("connected");
  init_Neopixel(50);
  initOLED();
  EEPROM.begin(EEPROM_SIZE);
  setupOTA("ESP_LED");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.write("WIFI_CONNECTED");

  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }

  // Register peer
  //esp_now_peer_info_t peerInfo = {};
  //memset(&peerInfo, 0, sizeof(peerInfo));

  peerInfo.channel = 0;  
  peerInfo.encrypt = false; // 암호화 ID/PW

  add_peer(wemos_lite);
  add_peer(esp32_38pin);
  add_peer(wemos_d1mini);

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.write("ESP_NOW_CONNECTED");
}



void loop() {
  reconnectWiFi();
  ArduinoOTA.handle();

  if(Serial.available())
  {  
      // packet 사이즈만큼 읽어옴
      Serial.readBytes((char*)&_data, sizeof(_data));
       _data.checksum += 1;
       neopixel_Flag = 1; 
      Serial.write((char*)&_data, sizeof(_data));
      pickOneLED(0, _data.RED, _data.GREEN, _data.BLUE, _data.brightness, _data.wait);
      delay(50);
  }   

  if( neopixel_Flag == 1 ){
    neopixel_Flag = 0;  

    String string_style = "";
    switch(_data.style)
    {
      case oneColor:
        string_style = "oneColor";
        poutput(string_style);
        pickOneLED(0, _data.RED, _data.GREEN, _data.BLUE, _data.brightness, _data.wait);
        break;
      case CHASE:
        string_style = "chase";
        poutput(string_style);
        break;  
        
    }
    
//    const char* format = "%d : [%d, %d, %d] style: %s, %d, %d %d";
//    sprintf(packet_buffer, format, _data.led_number,
//                                   _data.RED,
//                                   _data.GREEN,
//                                   _data.BLUE,
//                                   _data.brightness,
//                                   string_style,                                    
//                                   _data.wait,
//                                   _data.checksum );
//                                   
//    poutput(packet_buffer);
  }

}

void poutput(String _string){
  TelnetStream.println(_string);
  mySerial.println(_string);
}

void add_peer(uint8_t peer_name[]){
  /// register third peer
  memcpy(peerInfo.peer_addr, peer_name, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }else{
    Serial.println("Succeed to add peer");
  }
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:  ");
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
  // 받을 구조체, 버퍼(받은 데이터), 구조체 크기
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  Serial.print("Bytes received: ");
  Serial.println(len);

  incomingRGB[0] = incomingReadings.RED;
  incomingRGB[0] = incomingReadings.GREEN;
  incomingRGB[0] = incomingReadings.BLUE;
  

  Serial.print("수신 -> Red : ");
  Serial.println(incomingRGB[0]);
  Serial.print("Green : ");
  Serial.println(incomingRGB[1]);
  Serial.print("Blue : ");
  Serial.println(incomingRGB[2]);
 
}
