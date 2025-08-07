#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#include "qbuffer.h"

#define MYPORT_TX      12 // Blue
#define MYPORT_RX      13 // Green
#define LED_BUILTIN    2
#define MAX_DISTANCE   2500 // Read MAX 3000 mm
#define ERROR_DISTANCE 2550
#define ZERO_DISTANCE  2510
#define BELOW_ZERO_DISTANCE 50

uint32_t lastTime[2] = {0,};
uint32_t timerDelay = 100;
int counter = 0;
bool oledUpdate = false;

#define QBUFFER_SIZE 64 // At least 64 byte for 9600 baud rate
uint8_t qbuf_data[QBUFFER_SIZE];
qbuffer_t qbuf;

int distance;


EspSoftwareSerial::UART myPort;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  qbufferCreate(&qbuf, qbuf_data, QBUFFER_SIZE);
  delay(1000);

  myPort.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (!myPort) { 
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
    while (1) { 
      delay (1000);
    }
  } 

  u8g2.begin(); // SDA 21, SCL 22

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 10);
  u8g2.print("Hello World!");
  u8g2.sendBuffer();
  Serial.println("Start Sonar!");
  delay(1000);  
}

void loop() {

  uint32_t currentTime = millis();
  
  // Read Data if data comes
  while(myPort.available())
  {
    uint8_t sonarByte = myPort.read();
    qbufferWrite(&qbuf, &sonarByte, 1);
  }
  
  if(qbufferAvailable(&qbuf) >= 4)
  {
    uint8_t packet[4];
    qbufferRead(&qbuf, packet, 4);

    if (packet[0] == 0xFF) 
    {
      uint8_t checksum = (packet[0] + packet[1] + packet[2]) & 0xFF;
      if (checksum == packet[3])
      {
        distance = (packet[1] << 8) | packet[2];
        if (distance > BELOW_ZERO_DISTANCE) {   // 50 mm
          Serial.print("Distance: ");
          Serial.print(distance / 10.0);
          Serial.println(" cm");
          oledUpdate = true;
          digitalWrite(LED_BUILTIN, LOW);
        } else {
          Serial.println("Below lower limit");
          digitalWrite(LED_BUILTIN, HIGH);
          distance = ZERO_DISTANCE;
        }
      } 
      else 
      {
        Serial.println("Checksum error");
        distance = ERROR_DISTANCE;
      }
    }
  }

  if(currentTime - lastTime[0] >= 1000){
    lastTime[0] = currentTime;

    // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    counter++;
  }
  

  if (currentTime - lastTime[1] >= timerDelay || oledUpdate == true) {
    lastTime[1] = currentTime;
  
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10);
    u8g2.printf("Counter: %d", counter);
    u8g2.setCursor(0, 30);
    u8g2.printf("Sonar: %5.2f cm", distance / 10.0);
    u8g2.setCursor(0,45);
    u8g2.printf("Limit: 28 cm");
    
    u8g2.setCursor(0, 60);
    
    if(distance /10.0 == 255)
    {
      u8g2.printf("Error     ");
    }
    else if(distance / 10.0 == 251)
    {
      u8g2.printf("Below Zero");
    }
    u8g2.sendBuffer();
    oledUpdate = false;
  }
}




