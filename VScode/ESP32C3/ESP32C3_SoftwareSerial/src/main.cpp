#include <Arduino.h>

#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include "neopixel.h"

#define RX1_PIN 8
#define TX1_PIN 10

#define MYPORT_RX 6
#define MYPORT_TX 7

uint64_t count = 0;
uint64_t countTime = 0;

HardwareSerial mySerial(Serial1);
EspSoftwareSerial::UART myPort;

MyNeopixel* myNeoPixel = new MyNeopixel();


void setup(){
  Serial.begin(115200); // Standard hardware serial port
  myNeoPixel->InitNeopixel();
  mySerial.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  myPort.begin(115200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (!myPort) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
    while (1) { // Don't continue with invalid configuration
      delay (1000);
    }
  }
  myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(255, 0, 0), 50, 1);

  Serial.printf("\r\n- Start -\r\n");
  delay(1000);
}


void loop(){
  if(myPort.available() )
  {
    char text = myPort.read();
    if(text == 'a')
    {
      myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(0, 255, 0), 50, 1);
      Serial.println("Green");
      myPort.write("GREEN\r\n");
    }
    else if(text = 'b')
    {
      myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(0, 0, 255), 50, 1);
      Serial.println("Blue");
      myPort.write("BLUE\r\n");
    }
    else
    {

    }
  }

  if(mySerial.available() )
  {
    char text = mySerial.read();
    if(text == 'a')
    {
      myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(0, 255, 0), 50, 1);
      Serial.println("Green");
      mySerial.write("GREEN\r\n");
    }
    else if(text = 'b')
    {
      myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(0, 0, 255), 50, 1);
      Serial.println("Blue");
      mySerial.write("BLUE\r\n");
    }
    else
    {

    }
  }
  

  if(millis() - countTime >= 1000)
  {
    countTime = millis();
    count++;
    myPort.printf("Soft : %d\r\n",count);
    mySerial.printf("Hard : %d\r\n",count);
    Serial.printf("CNT : %d\r\n",count);

  }
  //delay(10);
//  myPort.printf("Hello SoftwareSerial\r\n");
//  delay(2000);

}
