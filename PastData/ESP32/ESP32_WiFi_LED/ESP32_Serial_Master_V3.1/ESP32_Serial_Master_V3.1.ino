#include "OTA.h"
#include "Neopixel.h"
#include <AsyncElegantOTA.h>
#include <esp_now.h>
#include <HardwareSerial.h>
 
HardwareSerial mySerial(2);

uint8_t wemos_d1mini[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
uint8_t esp32_38pin[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};
uint8_t wemos_lite[] = {0x78, 0xE3, 0x6D, 0x19, 0x2F, 0x44};
uint8_t esp_module_0[] = {0x58, 0xBF, 0x25, 0x17, 0x6D, 0x28};

String success;
uint8_t incomingRGB[3];

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

unsigned long RESET_TIME = 1000*60*60*24; // 1 day
volatile int count0;
volatile int count1; 
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

bool FORMAT_SPIFFS_IF_FAILED = true;    // 마운트 실패시 SPIFFS 파일 시스템 포맷

esp_now_peer_info_t peerInfo;
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
AsyncWebServer server(80);

void setup() {
  
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 26, 27);
  mySerial.println("connected");
  init_Neopixel(50);
  initWiFi();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Connection Test -> /update to upload bin file");
  });
  AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
  server.begin();
  Serial.println("HTTP server started");
 
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
  Serial.println();

  TelnetStream.begin(); // wifi 보다 아래에 위치해야함
  timer_init();
  
}


void loop() {
  reconnectWiFi(); 

  if(Serial.available())
  {  
      // packet 사이즈만큼 읽어옴
      Serial.readBytes((char*)&serial_data, sizeof(serial_data));
      serial_data.checksum += 1;
      neopixel_Flag = 1; 
      Serial.write((char*)&serial_data, sizeof(serial_data));
      pickOneLED(0, serial_data.RED, serial_data.GREEN, serial_data.BLUE, serial_data.brightness, serial_data.wait);
      delay(50);
          
 
  }

  if(serial_data.led_number == 255) 
  {
      server.end();
      pickOneLED(0, 255, 255, 255, 50, 50);
      print_output("Reset WiFi");
      changeWiFi();
      server.begin();
      ESP.restart();
  }

  
  if( neopixel_Flag == 1 ){
    
    neopixel_Flag = 0;  
    String string_style = "";
    
    switch(serial_data.style)
    {
      case oneColor:
        string_style = "oneColor";
        print_output(string_style);
        pickOneLED(0, serial_data.RED, serial_data.GREEN, serial_data.BLUE, serial_data.brightness, serial_data.wait);
        break;
      case CHASE:
        string_style = "chase";
        print_output(string_style);
        break;         
    }
  }


}


void print_output(String _string){
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



void IRAM_ATTR onTimer0() {
   portENTER_CRITICAL_ISR(&timerMux);
   count0++;
   portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timerMux);
   count1++;
   portEXIT_CRITICAL_ISR(&timerMux);
}


void timer_init(){
   // Configure the Prescaler at 80 the quarter of the ESP32 is cadence at 80Mhz
   // 80000000 / 80 = 1000000 tics / seconds  T: 0.000001s, 1초에 1,000,000 찍음
                 
   timer0 = timerBegin(0, 80, true);  
   timer1 = timerBegin(1, 80, true);        
   timerAttachInterrupt(timer0, &onTimer0, true);        
   timerAttachInterrupt(timer1, &onTimer1, true);  
    
   // Sets an alarm to sound every second
   //  number/1,000,000 -> 시간(s)   또는  원하는 시간/0.000001 -> 타이머 인터럽트가 발생되는 Count횟수
   timerAlarmWrite(timer0, 2000000, true);
   timerAlarmWrite(timer1, 30000000, true); 
   
   timerAlarmEnable(timer0);
   timerAlarmEnable(timer1);
}
