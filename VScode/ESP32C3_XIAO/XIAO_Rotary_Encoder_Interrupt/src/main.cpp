#include <Arduino.h>

//#define ENCODER_LIB

#ifdef ENCODER_LIB

#include <Encoder.h>

#endif

const int CLK = D8;
const int DT  = D9;
const int SW  = D10;



int counter = 0;
int buttonCounter = 0;
bool buttonState = false;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long long lastEncoderDelay = 0;
unsigned long long lastButtonPress = 0;

void IRAM_ATTR updateEncoder();
void IRAM_ATTR buttonPress();

#ifdef ENCODER_LIB
long oledPosition = -999;
Encoder myEnc(DT, CLK);

#endif

void setup() {
  Serial.begin(115200);

#ifndef ENCODER_LIB
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT); // 제품에 Pull-Up 처리 되어있음

  lastStateCLK = digitalRead(CLK);

  attachInterrupt(digitalPinToInterrupt(CLK), updateEncoder, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(DT), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SW), buttonPress, CHANGE);
#endif

}





void loop() {

#ifdef ENCODER_LIB
  long newPosition = myEnc.read();
  if(newPosition != oledPosition)
  {
    oledPosition = newPosition;
    Serial.println((String)"Position : " + newPosition);
  }
#endif

}


void IRAM_ATTR updateEncoder()
{
  if(millis() - lastEncoderDelay > 2)
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

    lastEncoderDelay = millis();  
  }

}

void IRAM_ATTR buttonPress()
{

  if(millis() - lastButtonPress > 100)
  {
    if(!buttonState)
    {
      buttonCounter ++;
      Serial.println((String)"Button Pressed [" + buttonCounter + "]");
    }
    else
    {
      // Because of Only Change Interrupt Works
    }

    buttonState = ! buttonState;
    lastButtonPress = millis();
  }    

}