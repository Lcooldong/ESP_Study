#include <Arduino.h>
#define HALL_VOUT   25
#define HALL_VOUT_2 26


void setup() {
  Serial.begin(115200);
}

void loop() {
  int vout = analogRead(HALL_VOUT);
  int vout2 = analogRead(HALL_VOUT_2);

  Serial.printf("Value : %4d  |  %4d\n" , vout, vout2);
  delay(100);
}

