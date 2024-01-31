#include <Arduino.h>
#include "neopixel.h"


MyNeopixel* myNeopixel = new MyNeopixel();
uint64_t lastTime = 0;
uint32_t interval = 1000;

void setup() {
  Serial.begin(115200);
  myNeopixel->InitNeopixel();
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 50);
  Serial.println("Start QT_Py");
}

void loop() {

  if(millis() - lastTime > interval)
  {
    lastTime = millis(); 
    Serial.println("Hello QT Py");

  }
  
}

