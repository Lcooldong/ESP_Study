#include <Arduino.h>
#include "Timer.h"

#define BUILTIN_LED 2

const unsigned long PERIOD1 = 500;
int count = 0;

Timer timer1;

void printHello();

void setup() {
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);

  //timer1.every(2000, printHello, -1);
  //timer1.every(2000, printHello, 4);
  timer1.after(3000, printHello);
  

  // timer1.oscillate(BUILTIN_LED, PERIOD1, HIGH); // forever

  // timer1.pulse(BUILTIN_LED, 5000, LOW);  // once 일정시간동안 !startValue 후에 startValue

  // timer1.pulseImmediate(BUILTIN_LED, 3000, HIGH); // once  일정시간동안 startValue 후에 !startValue


}

void loop() {
  timer1.update();
}

void printHello()
{
  Serial.printf("%d : Hello\r\n", count++);
}

