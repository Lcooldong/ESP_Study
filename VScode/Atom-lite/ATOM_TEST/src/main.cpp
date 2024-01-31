#include <Arduino.h>
#include "M5Atom.h"
#include "neopixel.h"

long count = 0;
MyNeopixel* myNeopixel = new MyNeopixel();

void setup() {
  M5.begin(false, true, false);
  Serial.begin(115200);
  myNeopixel->InitNeopixel();
  delay(50);
  Serial.println("Start Atom");
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 255), 50, 50);
}

void loop() {
  if(M5.Btn.wasPressed())
  {
    Serial.printf("Count : %d\r\n", count++);
    delay(50);
  }
  
  M5.update();  // M5.Btn.read();
}



