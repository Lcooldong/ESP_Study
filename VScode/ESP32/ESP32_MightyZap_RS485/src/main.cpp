#include <Arduino.h>
#include <MightyZap.h>

#define DIRECTION_HIGH 5
#define ID_NUM 0
#define SERVO_MAX_VALUE 4095
#define SERVO_MAX_SPEED 1023
#define LED_NONE  2
#define LED_GREEN 4
#define LED_RED   8

const int RX1_PIN = 6;
const int TX1_PIN = 7;

u_int64_t stateTime = 0;
bool directionToggle = false;

HardwareSerial RS485(Serial1);
Mightyzap linearServo(&RS485, DIRECTION_HIGH);



void setup() {
    Serial.begin(115200);

    RS485.setPins(RX1_PIN, TX1_PIN);
    // RS485.begin(57600, SERIAL_8N1, RX1_PIN, TX1_PIN);
    linearServo.begin(32);  // 57600



    pinMode(DIRECTION_HIGH, OUTPUT);
    digitalWrite(DIRECTION_HIGH, HIGH);

    linearServo.GoalSpeed(ID_NUM, SERVO_MAX_SPEED);
    linearServo.GoalPosition(ID_NUM, 0);
    linearServo.ledOn(ID_NUM, LED_GREEN);
}

void loop() {
  if(millis() - stateTime >= 3000)
  {
    stateTime = millis();

    if(!directionToggle)
    {
        linearServo.GoalPosition(ID_NUM, 0);
    }
    else
    {
        linearServo.GoalPosition(ID_NUM, SERVO_MAX_VALUE);
    }

    directionToggle = !directionToggle;
  }
}

