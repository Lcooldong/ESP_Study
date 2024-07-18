#include <Arduino.h>
const int PMODE_0 = GPIO_NUM_18;

void setup() {
  Serial.begin(115200);
  pinMode(PMODE_0, INPUT);
}

void loop() {
  // int value = analogRead(PMODE_0);
  int d_value = digitalRead(PMODE_0);
  Serial.printf("V : %d \r\n", d_value);
  delay(100);
}

