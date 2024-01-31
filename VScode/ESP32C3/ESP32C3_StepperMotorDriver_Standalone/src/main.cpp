#include <Arduino.h>

// /**
//  * Author Teemu Mäntykallio
//  * Initializes the library and runs the stepper
//  * motor in alternating directions.
//  */

#define SERVO_PIN 1
#define HALL_SENSOR_PIN 4

#define EN_PIN           0 // Enable
#define DIR_PIN          8  // Direction
#define STEP_PIN         10  // Step

#define STEPS 1600
#define STEP_DELAY 100


void setup() {
  Serial.setTimeout(500);
  Serial.begin(115200);
  
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);      // Enable driver in hardware

}

bool shaft = false;

void loop() {

  if(Serial.available())
  {
    char charText = Serial.read();
    switch (charText) {
    case '+':
      Serial.printf("%c => Move Forward\r\n", charText);
      digitalWrite(DIR_PIN, true);
      
      for (uint16_t i = STEPS; i>0; i--) 
      {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY);
      }
      break;
    case '-':
      Serial.printf("%c => Move Backward\r\n", charText);
      digitalWrite(DIR_PIN, false);
      for (uint16_t i = STEPS; i>0; i--) 
      {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY);
      }     
      break;
    case 'e':
      Serial.printf("Enable Stepping Driver\r\n");
      digitalWrite(EN_PIN, LOW);
      break;
    case 'd':
      Serial.printf("Disable Stepping Driver\r\n");
      digitalWrite(EN_PIN, HIGH);
      break;
    }

  }
}