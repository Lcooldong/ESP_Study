#include <IRROBOT_EZController.h>
#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

int Speed, Display=1,cPosition;

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
    Serial.print("New Speed[0~1023] : ");
    Display=0;
  }  
  if(Serial.available()) {
    Speed = Serial.parseInt();
    Easy.MightyZap.GoalSpeed(ID_NUM,Speed);
    Serial.println(Easy.MightyZap.GoalSpeed(ID_NUM));
    delay(50);
    Display=1;
  }
}
