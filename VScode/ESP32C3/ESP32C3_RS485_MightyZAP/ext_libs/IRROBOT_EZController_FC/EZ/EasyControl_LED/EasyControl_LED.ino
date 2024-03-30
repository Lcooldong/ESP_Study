#include <IRROBOT_EZController.h>

#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

int ForceLimit;

void setup() {
  Easy.MightyZap.begin(32);  
}

void loop() {     
  Easy.MightyZap.ledOn(ID_NUM,RED);
  delay(1000);  
  Easy.MightyZap.ledOn(ID_NUM,GREEN);  
  delay(1000);
}
