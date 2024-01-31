#include <Arduino.h>

#define LED_PIN 25

volatile uint32_t cnt = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println(cnt++);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}

