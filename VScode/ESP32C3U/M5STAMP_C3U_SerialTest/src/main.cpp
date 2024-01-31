#include <Arduino.h>
#include "neopixel.h"

// C3U 는 Serial 통신 X

MyNeopixel* myNeopixel = new MyNeopixel();

void setup() {
  Serial.begin(115200);
  myNeopixel->InitNeopixel();

}

void loop() {
  if(Serial.available())
  {
    char text = Serial.read();
    if(text == 't')
    {
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 25), 50, 1);
    }
    else
    {
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 255), 50, 1);
    }
  }
}

