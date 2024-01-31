#include <Arduino.h>

#include <SoftwareSerial.h>
#include "neopixel.h"

#define MYPORT_RX 6
#define MYPORT_TX 7


EspSoftwareSerial::UART myPort;
MyNeopixel* myNeoPixel = new MyNeopixel();


void setup(){
  Serial.begin(115200); // Standard hardware serial port
  myNeoPixel->InitNeopixel();

  myPort.begin(115200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (!myPort) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
    while (1) { // Don't continue with invalid configuration
      delay (1000);
    }
  }
  myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(255, 0, 0), 50, 1);

}


void loop(){
  if(myPort.available() )
  {
    char text = myPort.read();
    if(text == 'a')
    {
      myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(0, 255, 0), 50, 1);
      Serial.println("Get a");
      myPort.write("1");
    }
    else if(text = 'b')
    {
      myNeoPixel->pickOneLED(0, myNeoPixel->strip->Color(0, 0, 255), 50, 1);
      Serial.println("Get b");
      myPort.write("2");
    }
    else
    {

    }

  }
  delay(10);
//  myPort.printf("Hello SoftwareSerial\r\n");
//  delay(2000);

}
