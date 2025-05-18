#include <Arduino.h>

int count = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); // 21

}

void loop() {
  count++;
  Serial.printf("[%d]\r\n", count);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(1000);
}

