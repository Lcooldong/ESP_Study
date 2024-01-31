#include <Arduino.h>

#define Z_PIN   4
#define S0_PIN  5
#define S1_PIN  6
#define S2_PIN  7
#define EN_PIN  8

uint32_t lastTime = 0;
uint32_t interval = 1000;

void setup() {
  Serial.begin(115200);

  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
}

void loop() {
  if(Serial.available())
  { 
    char charText = Serial.read();
    
    switch (charText)
    {
    case '0':
      digitalWrite(S0_PIN, LOW);
      digitalWrite(S1_PIN, LOW);
      break;
    case '1':
      digitalWrite(S0_PIN, HIGH);
      digitalWrite(S1_PIN, LOW);  
      break;

    case '2':
      digitalWrite(S0_PIN, LOW);
      digitalWrite(S1_PIN, HIGH);
      break;

    case '3':
      digitalWrite(S0_PIN, HIGH);
      digitalWrite(S1_PIN, HIGH);
      break;

    default:
      
      break;
    }

  }

  if (millis() - lastTime > interval)
  {
    lastTime = millis();
    Serial.printf("Value : %d\r\n", analogRead(Z_PIN));
  }
  
}

