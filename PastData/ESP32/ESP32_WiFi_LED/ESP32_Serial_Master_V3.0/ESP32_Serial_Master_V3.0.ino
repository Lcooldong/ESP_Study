#include "OTA.h"
#include <AsyncElegantOTA.h>
#include <esp_now.h>
#include <HardwareSerial.h>
 
HardwareSerial mySerial(2);

unsigned long RESET_TIME = 1000*60*60*24; // 1 day
volatile int count0;
volatile int count1; 
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

bool FORMAT_SPIFFS_IF_FAILED = true;    // 마운트 실패시 SPIFFS 파일 시스템 포맷


AsyncWebServer server(80);

void setup() {
  
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 26, 27);
  mySerial.println("connected");
  initWiFi();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Connection Test -> /update to upload bin file");
  });
  AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
  server.begin();
  Serial.println("HTTP server started");
 
  TelnetStream.begin();

  timer_init();
  
}


void loop() {
  reconnectWiFi(); 

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
