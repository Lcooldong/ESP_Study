#include <Arduino.h>
#include <Servo.h>
#include <stdio.h>
#include "stdlib.h"

#define MG996R_PIN 28
#define HEARTBEAT_INTERVAL  1000

Servo myServo;

int currentPos = 0;

uint64_t currentTime = 0;
uint64_t lastTime = 0;
int cnt = 0;

void setup() {

  Serial.begin(115200);
  
  myServo.attach(MG996R_PIN);
 //myServo.setPeriodHertz(500);
 //myServo.attach(MG996R_PIN, 1000, 2000);
}

void loop() {

  
  if(millis() - lastTime > HEARTBEAT_INTERVAL)
  {
    lastTime = millis();
    Serial.println(cnt++);
  }

  if(Serial.available())
  {
    char c = Serial.read();
    switch (c)
    {
    case 'u':
      currentPos += 1;
      break;
    
    case 'd':
      currentPos -= 1;
      break;

    default:
     myServo.write(currentPos);
      delay(1);
      break;
    }

  }

}

