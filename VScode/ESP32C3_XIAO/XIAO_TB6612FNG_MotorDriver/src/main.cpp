#include <Arduino.h>

// #define FPWM 100000
#define FPWM 5000
#define RESOLUTION 10 // 0 ~ 1023

const int POT = D0;

const int PWMA = D2;
const int PWMB = D3;

const int AIN1 = D4;
const int AIN2 = D5;
const int BIN1 = D8;
const int BIN2 = D9;

const int STBY = D10;

const int PWMA_CHANNEL = 0;
const int PWMB_CHANNEL = 1;

uint64_t lastPotentionMeter = 0;
uint16_t _speed = 0;
bool _direction = true;


uint64_t lastMoveMillis = 0;
uint64_t lastPwmMillis = 0;
bool motorState = false;
uint16_t pwmValue = 0;
bool pwmState = true;

enum
{
  MOTOR_A = 0,
  MOTOR_B = 1
};

typedef struct _MOTOR
{
  int _name = 0;
  uint16_t _speed = 0;
  bool _direction = true;
  int _offset = 0;
}MOTOR;

MOTOR motorA;
MOTOR motorB;

void move(int motorName , uint16_t speed, bool direction);

void setup() {
  Serial.begin(115200);
  pinMode(POT, INPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  // pinMode(PWMA, OUTPUT);
  // pinMode(PWMB, OUTPUT);

  ledcSetup(PWMA_CHANNEL, FPWM, RESOLUTION);  
  ledcAttachPin(PWMA, PWMA_CHANNEL);
  
  ledcSetup(PWMB_CHANNEL, FPWM, RESOLUTION);
  ledcAttachPin(PWMB, PWMB_CHANNEL);


  // digitalWrite(AIN1, LOW);
  // digitalWrite(AIN2, LOW);
  // digitalWrite(BIN1, LOW);
  // digitalWrite(BIN2, LOW);
  // digitalWrite(STBY, LOW);

  // ledcWrite(PWMA_CHANNEL, 0);
  // ledcWrite(PWMB_CHANNEL, 0);

}

void loop() {

  int potValue = analogRead(POT);

  if(millis() - lastPotentionMeter > 100)
  {
    if(potValue > 20)
    {
      _speed = map(potValue, 0, 4095, 0, pow(2, RESOLUTION) - 1);
    }
    else
    {
      _speed = 0;
    }
    // Serial.println((String)"Value : " + potValue + "|" + _speed);
    lastPotentionMeter = millis();
  }

  if(millis() - lastMoveMillis > 1000)
  { 
    move(MOTOR_A, _speed, motorState);
    motorState = ! motorState;
    
    lastMoveMillis = millis();
  }

  if(millis() - lastPwmMillis > 10)
  {

    if(pwmValue >= 512) // MAX 1023
    {
      pwmState = false;
    }
    else if(pwmValue <= 0)
    {
      pwmState = true;
    }

    if(pwmState)
    {
      pwmValue++;
    }
    else
    {
      pwmValue--;
    }

    move(MOTOR_B, pwmValue, pwmState);
    lastPwmMillis = millis();
  }
  

}




void move(int motorName , uint16_t speed, bool direction)
{ 
  digitalWrite(STBY, HIGH);

  if(motorName == MOTOR_A)
  {
    if(direction)
    {
      Serial.print("Forward  ->");
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
    }
    else
    {
      Serial.print("Backward ->");
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
    }
    motorA._direction = direction;
    motorA._speed = speed;
    Serial.println((String)" Speed : " + motorA._speed);
    ledcWrite(PWMA_CHANNEL, motorA._speed);
  }
  else if(motorName == MOTOR_B)
  {
    if(direction)
    {
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
    }
    else
    {
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
    }
    motorB._direction = direction;
    motorB._speed = speed;
    ledcWrite(PWMB_CHANNEL, motorB._speed);
  }

  
}