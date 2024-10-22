#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
// put function declarations here:

#define RS485 Serial1

// EspSoftwareSerial::UART RS485;
unsigned long rs485Time  = 0 ;
unsigned long lastTime = 0;
const int RS485_RX = D0;
const int RS485_TX = D1;
int lightValue = 0;

int count = 0;

void rs485Command();

void setup() {
  Serial.begin(115200);
  RS485.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  // RS485.begin(9600, EspSoftwareSerial::SWSERIAL_8N1, RS485_RX, RS485_TX);
}

void loop() {
  if(millis() - lastTime > 1000)
  {
    count++;
    RS485.printf("[%d]\r\n", count);
    Serial.printf("[%d]\r\n", count);
    lastTime = millis();

  }

  rs485Command();
}

void rs485Command()
{
  if(millis() - rs485Time > 1)
  {
    if(RS485.available())
    {
      char cmd = RS485.read();

      switch (cmd)
      {
      case '1':
        Serial.printf("RS485 : %c\r\n", cmd);
        
        break;
      
      case '2':
        Serial.printf("RS485 : %c\r\n", cmd);
        
        break;
      case '3':        
        if(lightValue >= 255)
        {
          lightValue = 255;
        }
        else
        {
          lightValue++;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, lightValue);
        // ledcWrite(ledChannel2 , lightValue);
        break;
      case '4':
        if(lightValue <= 0)
        {
          lightValue = 0;
        }
        else
        {
          lightValue--;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, lightValue);
        // ledcWrite(ledChannel2 , lightValue);
        break;

      default:
        break;
      }

    }
    rs485Time = millis();
  }

}