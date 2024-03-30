#include <IRROBOT_EZController.h>

#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

void setup() {
  Easy.MightyZap.begin(32);  
}

void loop() {     
  Easy.MightyZap.LongStrokeLimit(ID_NUM,4095);  
  Easy.MightyZap.GoalPosition(ID_NUM,4095);      
  delay(5000);
   
  Easy.MightyZap.ShortStrokeLimit(ID_NUM,0);  
  Easy.MightyZap.GoalPosition(ID_NUM,0);         
  delay(5000);   
}
