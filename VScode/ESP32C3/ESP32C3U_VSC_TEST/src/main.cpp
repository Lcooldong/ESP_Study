#include "main.h"

int cnt = 0;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  Serial.println("ESP32C3U");
  pinMode(BTN, INPUT_PULLUP);
 
}

void loop() {
  int pushed = digitalRead(BTN);
  Serial.println(cnt ++);
  delay(1000);
  
  if (pushed == 0){
    Serial.println("1 Seconds");
    delay(100);
  }
  
}