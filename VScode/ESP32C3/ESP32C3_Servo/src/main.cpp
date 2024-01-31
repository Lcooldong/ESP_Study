#include <Arduino.h>
#include "neopixel.h"
#include <ESP32Servo.h>

#include <Wire.h>
#include <SPI.h> 


#define SERVO_PIN 1
#define HALL_SENSOR_PIN 4

#define I2C_SCL 7
#define I2C_SDA 8

#define INTERVAL 50
#define HALL
#define SERVO_TARGET_POS 150
#define HALL_TARGET_VALUE 1280

long count = 0;
int hallCount = 0;
int pos = 0;
MyNeopixel* myNeopixel = new MyNeopixel();
Servo gripperServo;
bool ledToggle = false;
uint64_t lastTime = 0;
uint16_t hallValue;

#pragma pack(push, 1)
typedef struct packet_
{
  uint8_t STX;
  uint8_t _servo_state;
  uint8_t _fsr_state;
  uint8_t ETX;

}PACKET;
#pragma pack(pop)

PACKET testPACKET = {0,};
PACKET sendPACKET = {0,};

// 패킷 전송 미완성
const byte structSize = sizeof(PACKET);

// union packetData
// {
//   PACKET dataOut;
//   char buffer[structSize];
// };

// packetData u_packet;

struct __attribute__ ((packed)) testPacket
{

};



void PrintPacket(PACKET* _packet)
{
  Serial.printf("Prepare PACKET[%d]\r\n", sizeof(*_packet));
  Serial.println("-------------------------------------");
  for (int i = 0; i < 6; i++)
  {
    Serial.printf("=> %c \r\n", _packet[i]);
  }
  Serial.println("-------------------------------------");

}


PACKET PreparePacket(PACKET* _packet, uint8_t _servo_state, uint8_t _fsr_state)
{
  //PACKET _temp = {0,};

  _packet->STX = 0x02;
  _packet->_servo_state = _servo_state;
  _packet->_fsr_state = _fsr_state;
  _packet->ETX = 0x03;
  PrintPacket(_packet);

  return *_packet;
}


// void send(const void *data, size_t len)
// {
//   auto bytes = reinterpret_cast<const unsigned char *>(data);
//   while(len-- > 0)
//   {
//     Serial.print(*bytes++);
//   }
// }


void setup() {

  Serial.setTimeout(500);
  Serial.begin(115200);
  myNeopixel->InitNeopixel();
 

  if(!gripperServo.attached())
  {
    gripperServo.setPeriodHertz(50);
    gripperServo.attach(SERVO_PIN, 1000, 2000);
  }
  gripperServo.write(0);
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255), 50, 50);
}

uint32_t color = 0x000000;      // 'On' color (starts red)
int cnt = 0;
bool led_state = false;

void loop() {
  if(millis() - lastTime > INTERVAL)
  {
    lastTime = millis();
    
    hallValue = analogRead(HALL_SENSOR_PIN);
    Serial.printf("Value : %d\r\n", hallValue);
    if (hallValue <= HALL_TARGET_VALUE)
    {
      hallCount++;
      if(hallCount > 10)
      {
        Serial.println("Arrived at Target Height");
      }
    }
    else    
    {
      hallCount = 0;
    }
  }
  // UART
  if(Serial.available())
  {
    char charText = Serial.read();

    switch (charText)
    {
    case 'i':
      Serial.printf("%d\r\n", hallValue);
      break;

    case 'o':      
      if (pos == 0)
      {
        Serial.print("Servo Open Start\r\n");
        //PreparePacket(&sendPACKET ,0x65, 0x66);
        //PrintPacket(&sendPACKET);
        myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0), 50, 50);
        for (int i = 0; i <= SERVO_TARGET_POS; i++)
        {
          gripperServo.write(i);
          pos = i;
          //Serial.printf("Degree : %d\r\n", i);
          delay(2);
        }

        //Serial.write((byte*)&sendPACKET, sizeof(sendPACKET));
        
        Serial.printf("Servo Opened\r\n");
      }
      break;
    case 'c':
      
      if (pos == SERVO_TARGET_POS)
      {
        Serial.print("Servo Close Start\r\n");
        //PreparePacket(&sendPACKET ,0x01, 0x00);
        myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 50);
        for (int i = SERVO_TARGET_POS; i >= 0; i--)
        {
          gripperServo.write(i);
          pos = i;
          //Serial.printf("Degree : %d\r\n", i);
          delay(2);
        }
        Serial.printf("Servo Closed\r\n");
      }
      break;
    case 't':
      
      testPACKET = PreparePacket(&sendPACKET ,0x01, 0x00);  
      //PACKET temp = PreparePacket(1);
      //memcpy(&sendPACKET, &temp, sizeof(sendPACKET));
        
      Serial.printf("0x%02x | 0x%02x | 0x%02x \r\n",sendPACKET.STX, sendPACKET._servo_state ,sendPACKET.ETX);
      
      break;
    
    default:
      break;
    }




  }  

}
  



