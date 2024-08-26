#include <Arduino.h>


const int CLK = D8;
const int DT  = D9;
const int SW  = D10;



int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long long lastEncoderDelay = 0;
unsigned long long lastButtonPress = 0;


void setup() {
  Serial.begin(115200);

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT); // 제품에 Pull-Up 처리 되어있음

  lastStateCLK = digitalRead(CLK);
}

void loop() {
  
  if(millis() - lastEncoderDelay > 1)
  {
    currentStateCLK = digitalRead(CLK);

    if(currentStateCLK != lastStateCLK && currentStateCLK == 1)
    {
      if(digitalRead(DT) != currentStateCLK)
      {
        counter ++;
        currentDir = "CW  [->]";
      }
      else
      {
        counter--;
        currentDir = "CCW [<-]";
      }

      Serial.print("Dir : " + currentDir);
      Serial.println((String)" | Counter : " + counter);
    }

    lastStateCLK = currentStateCLK;

    int btnState  = digitalRead(SW);

    if(btnState == LOW)
    {
      if(millis() - lastButtonPress > 50)
      {
        Serial.println("Button Pressed");
      }

      lastButtonPress = millis();
    }

    lastEncoderDelay = millis();
  }

 

}

