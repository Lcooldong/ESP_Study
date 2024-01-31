#include <Arduino.h>
#include <esp_now.h>

#define INTERVAL 1000





typedef struct __attribute__((packed)) packet
{
  uint8_t stx;
  uint8_t servoState;
  uint8_t etx;
}PACKET;

PACKET dataToSend = {0,};
PACKET buf;

uint64_t lastTime = 0;

void initPacket(PACKET* _packet);
bool sendPacket(uint8_t* _data, size_t len);

void setup() {
  Serial.begin(115200);
  initPacket(&dataToSend);
}

void loop() {
  if(millis() - lastTime > INTERVAL)
  {
    lastTime = millis();
    //Serial.println("--PACKET--");
    
    
    //Serial.printf("0x%02x 0x%02x\r\n", dataToSend.stx, dataToSend.etx);
    

  }


  if(Serial.available())
  {
    char c = Serial.read();
    switch (c)
    {
    case 'i':
      
      //Serial.write('a');
      sendPacket((uint8_t*)&dataToSend, sizeof(dataToSend));
      //Serial.write(dataToSend);
      break;
    
    default:
      break;
    }
    delay(1);
  }
}



void initPacket(PACKET* _packet)
{
  _packet->stx = 0x02;
  _packet->etx = 0x03;
}


bool sendPacket(uint8_t* _data, size_t len)
{

  for (int i = 0; i < sizeof(buf); i++)
  {
    //Serial.printf("0x%x \r\n", _data[i]);
    Serial.write(_data[i]);
  }
  

  return true;
}

