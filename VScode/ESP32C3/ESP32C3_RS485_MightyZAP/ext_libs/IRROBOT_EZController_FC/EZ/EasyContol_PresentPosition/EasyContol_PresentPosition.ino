#include <IRROBOT_EZController.h>

#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

int Position;
int cPosition;
int Display =1;


void setup() {
  Serial.begin(9600);  
  Easy.MightyZap.begin(32);  
  while (! Serial);
  Easy.MightyZap.GoalSpeed(ID_NUM,100);
}

void loop() {     
  if(Display == 1){
    Serial.print("*New Position[0~4095] : ");
    Display = 0;
  }
  if(Serial.available())  {
    Position = Serial.parseInt(); 
    Serial.println(Position);    
    delay(200);

    Easy.MightyZap.GoalPosition(ID_NUM,Position);
    delay(50);
    while(Easy.MightyZap.Moving(ID_NUM)) {
      cPosition = Easy.MightyZap.presentPosition(ID_NUM);
      Serial.print("  - Position : ");
      Serial.println(cPosition);
    }   
    delay(50);
    cPosition = Easy.MightyZap.presentPosition(ID_NUM);
    Serial.print("  - final Position : ");
    Serial.println(cPosition);
    Display = 1;
  }  
}
