#include <Arduino.h>
#include <SimpleFOC.h>

const int pwmMin = 4;
const int pwmMax = 904;
const uint8_t encorderPin = 7;

//  PinPWM / min / max
MagneticSensorPWM sensor1 = MagneticSensorPWM(encorderPin, pwmMin, pwmMax);


uint64_t pwmLastTime = 0;
void doPWM1(){sensor1.handlePWM();}
void setup() {
  Serial.begin(115200);

  sensor1.init();
  sensor1.enableInterrupt(doPWM1);
}

void loop() {
  // if(millis() - pwmLastTime > 100)
  // {
  //   Serial.printf("Value : %f\r\n" , pulseIn(encorderPin, HIGH));
  //   pwmLastTime = millis();
  // }

  sensor1.update();
  Serial.print(sensor1.getAngle());
  Serial.println();
}

