#include <esp_now.h>
#include "OTA.h"
#include "Neopixel.h"
#include <AsyncElegantOTA.h>

unsigned long RESET_TIME = 1000*60*60*24; // 1 day
volatile int count0;  //extern OTA.cpp
volatile int count1;  //extern OTA.cpp
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

String success;
uint8_t incomingRGB[3];

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


bool FORMAT_SPIFFS_IF_FAILED = true;
AsyncWebServer server(80);

//////////////////////////////////////////////////////////
int device_id = 1;    // check
/////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
   
  initWiFi();
  // Init ESP-NOW
  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Connection Test -> /update to upload bin file");
  });
  AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
  server.begin();
  Serial.println("HTTP server started");


  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  timer_init();

  
}

void loop() {
  reconnectWiFi();
  
  
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
  TelnetStream.print("Bytes received: ");
  TelnetStream.println(len);
  
  incomingRGB[0] = incomingReadings.RED;
  incomingRGB[1] = incomingReadings.GREEN;
  incomingRGB[2] = incomingReadings.BLUE;
  
  Serial.print("Device_LED : ");
  Serial.println(incomingReadings.device_led);

  Serial.print("Red : ");
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

  
  
  TelnetStream.print("Device_LED : ");
  TelnetStream.println(incomingReadings.device_led);

  TelnetStream.print("Red : ");
  TelnetStream.println(incomingRGB[0]);
  TelnetStream.print("Green : ");
  TelnetStream.println(incomingRGB[1]);
  TelnetStream.print("Blue : ");
  TelnetStream.println(incomingRGB[2]);
  TelnetStream.print("Brightness : ");
  TelnetStream.println(incomingReadings.brightness);
  TelnetStream.print("Sytle : ");
  TelnetStream.println(incomingReadings.style);
  TelnetStream.print("wait Time : ");
  TelnetStream.println(incomingReadings.wait);
  TelnetStream.print("checksum : ");
  TelnetStream.println(incomingReadings.checksum);


  uint8_t target_board_led = incomingReadings.device_led;
  uint8_t R = incomingReadings.RED;
  uint8_t G = incomingReadings.GREEN;
  uint8_t B = incomingReadings.BLUE;
  uint8_t _brightness = incomingReadings.brightness;
  uint8_t waitORtimes = incomingReadings.wait;
  strip.setBrightness(_brightness);
  
  // target_board_led >> 4
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

  
    if(incomingReadings.style == 255){
      server.end();
      Serial.println("Reset WiFi"); 
      TelnetStream.println("Reset WiFi"); 
      changeWiFi();
      server.begin();     
    }
  }

  
  
}

void resetNeopixel(){
  for(int i=0; i < 256; i++){
    pickOneLED(i, strip.Color(0, 0, 0), 0, 0 );
  } 
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
   timerAlarmWrite(timer0, 30000000, true);  // 1s
   timerAlarmWrite(timer1, 500000, true); // 15s
   
   timerAlarmEnable(timer0);
//   timerAlarmEnable(timer1);
}
