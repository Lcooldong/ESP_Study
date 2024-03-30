#include <IRROBOT_EZController.h>

#define ID_NUM 0

IRROBOT_EZController Easy(&Serial1);

void setup() {
  Serial.begin(9600);    
  Easy.MightyZap.begin(32);  
  while (! Serial);    
}

void loop() {     
  if(Serial.available())  {    
    char ch = Serial.read();
    if(ch=='d')    {
      Serial.print("Model Number        : ");  Serial.println((unsigned int)Easy.MightyZap.getModelNumber(ID_NUM));
      Serial.print("Firmware Version    : ");  Serial.println(Easy.MightyZap.Version(ID_NUM)*0.1);           
      Serial.print("Present Temperature : ");  Serial.println(Easy.MightyZap.presentTemperature(ID_NUM));
    }
  }
}
