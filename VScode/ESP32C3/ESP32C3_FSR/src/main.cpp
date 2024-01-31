#include <Arduino.h>

#define M5STAMP
//#define XIAO

#ifdef M5STAMP
  #define MOSFET_PIN 6
  #define LED_PIN 2
  #define BTN_PIN 3
#elif XIAO
  #define MOSFET_PIN 3
#endif

#define FSR_PIN_1 4
#define FSR_PIN_2 5
#define FSR_CUTOFF 3000

#define ESP32_RESET_VALUE 4000000000
 
bool OneTimeFlag = false;
unsigned int interval = 500;
unsigned long lastTime = 0; 

bool getFSR_INPUT();
void press_BTN();

void setup() {
  Serial.begin(115200);
  Serial.println("Hello");

  pinMode(FSR_PIN_1, INPUT_PULLUP);
  pinMode(FSR_PIN_2, INPUT_PULLUP);
  pinMode(MOSFET_PIN, OUTPUT);

  digitalWrite(MOSFET_PIN, LOW);

#ifdef M5STAMP
  pinMode(BTN_PIN, INPUT_PULLUP);
#endif

}


void loop() {
  bool result = getFSR_INPUT();
  
  if (millis() - lastTime > interval)
  {
    lastTime = millis();
    //Serial.println(result);
    if(result)
    {
      delay(1000);
      digitalWrite(MOSFET_PIN, HIGH);   // 방향 전환
      digitalWrite(MOSFET_PIN, LOW);
      Serial.println("Send to ControlBox");
      delay(1000);
    }
    else
    {
      digitalWrite(MOSFET_PIN, LOW);
    }
  }
  else
  {
#ifdef M5STAMP
  press_BTN();
     
#endif
  }



  if (millis() > ESP32_RESET_VALUE)
  {
    ESP.restart();
  }

}


bool getFSR_INPUT() {
  int FSR_1 = analogRead(FSR_PIN_1);
  int FSR_2 = analogRead(FSR_PIN_2);

  if((FSR_1 > FSR_CUTOFF) && (FSR_2 > FSR_CUTOFF))
  {
    return true;
  }
  else
  {
    return false;
  }
}


void press_BTN(){
  if(digitalRead(BTN_PIN) == false && OneTimeFlag)
  {
    Serial.println("pressed");
    while(true)
    {
      if (digitalRead(BTN_PIN) == true)
      {
        Serial.println("Released");
        break;
      }
    }
    OneTimeFlag = false;
  }
  else
  {
    OneTimeFlag = true;
  }

}
