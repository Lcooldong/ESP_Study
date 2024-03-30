#include <IRROBOT_EZController.h>

#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

int ForceLimit;

void setup() {
  Easy.MightyZap.begin(32);  
}

void loop() {     
  Easy.MightyZap.GoalPosition(ID_NUM, 0); //ID 0 MightZap moves to position 0
  delay(3000);
  Easy.MightyZap.GoalPosition(ID_NUM, 4095);//ID 0 MightZap moves to position 4095
  delay(3000);
}
