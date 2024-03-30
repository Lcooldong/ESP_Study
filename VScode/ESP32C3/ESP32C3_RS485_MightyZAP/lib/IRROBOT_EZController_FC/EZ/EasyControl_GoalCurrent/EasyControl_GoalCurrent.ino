#include <IRROBOT_EZController.h>
#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

int Current, Display=1,cPosition;

void setup() {
  Serial.begin(9600);  
  Easy.MightyZap.begin(32);  
  while (! Serial);
}

void loop() {     
  if(Display ==1) {                  
    Easy.MightyZap.GoalPosition(ID_NUM,0);
    delay(100);
    while(Easy.MightyZap.Moving(ID_NUM)){
      cPosition = Easy.MightyZap.presentPosition(ID_NUM);
      Serial.print("  - Position : ");
      Serial.println(cPosition);
    }    
    Easy.MightyZap.GoalPosition(ID_NUM,4095);
    delay(100);
    while(Easy.MightyZap.Moving(ID_NUM)){
      cPosition = Easy.MightyZap.presentPosition(ID_NUM);
      Serial.print("  - Position : ");
      Serial.println(cPosition);
    }
    Serial.print("New Current[0~1023] : ");
    Display=0;
  }  
  if(Serial.available()) {
    Current = Serial.parseInt();
    Easy.MightyZap.GoalCurrent(ID_NUM,Current);
    Serial.println(Easy.MightyZap.GoalCurrent(ID_NUM));
    delay(50);
    Display=1;
  }
}
