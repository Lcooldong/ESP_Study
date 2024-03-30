#include <IRROBOT_EZController.h>

#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

int ID_Sel =0;

void setup() {
  Serial.begin(9600);  
  Easy.MightyZap.begin(32);  
  while (! Serial);
  Serial.print("Input ID : ");  
}

void loop() {     
  Easy.MightyZap.ledOn(1,RED);
  delay(500);
  Easy.MightyZap.ledOn(2,GREEN);
  delay(500);
  if(Serial.available()) {  
    ID_Sel = Serial.parseInt();
    Serial.println(ID_Sel);
    Serial.print("Input_ID [0~3] : ");
    Easy.MightyZap.ServoID(0,ID_Sel);
    Easy.MightyZap.ServoID(1,ID_Sel);
    Easy.MightyZap.ServoID(2,ID_Sel);
  }
}
