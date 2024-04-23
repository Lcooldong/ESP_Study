#include <Arduino.h>
#include <IRROBOT_EZController.h>

#include <MightyZap.h>

#define ID_MAX 11
#define DATA_ENABLE_PIN 2
#define SERVO_CH1_PIN 5

IRROBOT_EZController Tester(&Serial1);
HardwareSerial hwSerial(Serial1);


int step_max = ID_MAX;
short step_value_min = 0;
short step_value_max = 1023;

Servo servo_CH1;

void setup() {
  
  Serial.begin(115200);
  hwSerial.begin(115200, SERIAL_8N1, 8, 7);
  // servo_CH1.attach(SERVO_CH1_PIN);

  // Mightyzap MightyZap(&hwSerial, DATA_ENABLE_PIN);
  // MightyZap.begin(32);
  
  // //Tester.begin();
  // //Tester.MightyZap.begin(32);   // Motor Baudrate 32 -> 57600 (default)  , 16 -> 115200
  // //Tester.setStep(ID_MAX, 0, 1023);

  // // MightyZap.GoalPosition(0, 0);
  // Serial.printf("%s\r\n", MightyZap.readRaw());
  Serial.printf("Done\r\n");
  delay(1000);
}

void loop() {
  // MightyZap.GoalPosition(0, 0);

  // delay(1000);
  // MightyZap.GoalPosition(0, 1000);
  // delay(1000);

  // Serial.printf("%s\r\n", MightyZap.readRaw());
  int ch = hwSerial.read();
  if(ch != -1)
  {
    // if((char)ch != '\n')
    // {
      Serial.printf("|%c", (char)ch);
    // }
  }

}

