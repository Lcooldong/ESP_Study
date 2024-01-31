#include <Arduino.h>
#include "neopixel.h"

unsigned long lastTime = 0;
unsigned long interval = 1;

MyNeopixel* neopixel = new MyNeopixel();


void setup() {
  Serial.begin(115200);
  neopixel->InitNeopixel();
}

void loop() {
  if(Serial.available())
  {
    String incommingData;
    incommingData = Serial.readStringUntil('\n');
      Serial.printf("Data %s\n", incommingData);

      if(incommingData == "Hello")
      {
        neopixel->pickOneLED(0, neopixel->strip->Color(0, 255, 0), 1, 5);
      }
      else if(incommingData == "Bye")
      {
        neopixel->pickOneLED(0, neopixel->strip->Color(255, 0, 0), 1, 5);
      }
      
    if(millis() - lastTime > interval)
    {
      
      lastTime = millis();      
    }
    
  }
  // put your main code here, to run repeatedly:
}

