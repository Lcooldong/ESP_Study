#include <ESP8266WiFi.h>
#define LED D6  // GPIO

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

}

void loop() {
  analogWrite(LED, 100);
  delay(1000);
  analogWrite(LED, 150);
  delay(1000);
  analogWrite(LED, 250);
  delay(1000);
  
}
