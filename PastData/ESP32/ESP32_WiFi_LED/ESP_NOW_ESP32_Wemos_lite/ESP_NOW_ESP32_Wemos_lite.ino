#include <esp_now.h>
#include <WiFi.h>
#include "Neopixel.h"

uint8_t wemos_d1mini[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
uint8_t esp32_38pin[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};
uint8_t wemos_lite[] = {0x78, 0xE3, 0x6D, 0x19, 0x2F, 0x44};
uint8_t esp_module_0[] = {0x58, 0xBF, 0x25, 0x17, 0x6D, 0x28};

String success;
uint8_t incomingRGB[3];

unsigned long t = 0;

#pragma pack(push, 1)
typedef struct _packet
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
  RAINBOW,
  COLORWIPE,
  CHASE_RAINBOW
}STYLE_Typedef;

PACKET serial_data = {0, };
PACKET incomingReadings;

int neopixel_Flag = 0;
extern Adafruit_NeoPixel strip; // 클래스 괄호는 초기화
esp_now_peer_info_t peerInfo;

const int device_id = 1;

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
  peerInfo.channel = 1;
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

  init_Neopixel(50);
}

void loop() {

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
  
  Serial.print("Device_LED : ");
  Serial.println(incomingReadings.device_led);

  Serial.print("수신 -> Red : ");
  Serial.println(incomingRGB[0]);
  Serial.print("Green : ");
  Serial.println(incomingRGB[1]);
  Serial.print("Blue : ");
  Serial.println(incomingRGB[2]);
  Serial.print("Brightness : ");
  Serial.println(incomingReadings.brightness);
  Serial.print("Sytle : ");
  Serial.println(incomingReadings.style);
  Serial.print("wait Time : ");
  Serial.println(incomingReadings.wait);
  Serial.print("checksum : ");
  Serial.println(incomingReadings.checksum);

  uint8_t target_board_led = incomingReadings.device_led;
  uint8_t R = incomingReadings.RED;
  uint8_t G = incomingReadings.GREEN;
  uint8_t B = incomingReadings.BLUE;
  uint8_t _brightness = incomingReadings.brightness;
  uint8_t waitORtimes = incomingReadings.wait;
  strip.setBrightness(_brightness);
  
  if(device_id == (target_board_led / 16))
  {
    switch(incomingReadings.style)
    {
      case oneColor:
        pickOneLED(target_board_led%16, strip.Color(R, G, B), _brightness, waitORtimes);
        break;
        
      case CHASE:
        theaterChase(strip.Color(R, G, B), waitORtimes);
        resetNeopixel();
        break;
        
      case RAINBOW:
        rainbow(waitORtimes);
        resetNeopixel();
        break;
        
      case COLORWIPE:
        colorWipe(strip.Color(R, G, B), waitORtimes * 10);
        break;
      case CHASE_RAINBOW:
        theaterChaseRainbow(waitORtimes);
        resetNeopixel();
        break;

      default:
        resetNeopixel();
        break;
    }  
  
  }
  
}

void resetNeopixel(){
  for(int i=0; i < 6; i++){
    pickOneLED(i, strip.Color(0, 0, 0), 0, 0 );
  } 
}
