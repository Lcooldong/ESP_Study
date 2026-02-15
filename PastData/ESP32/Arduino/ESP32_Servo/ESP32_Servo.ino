#include <ESP32_Servo.h>

// 서보모터 GND 와 보드 GND 연결
// 외부전원 사용 필수

static const int servoPin = 27;

Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin);
}

void loop() {
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees); // 모터의 각도를 설정합니다.
        Serial.println(posDegrees);
        delay(1);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees); // 모터의 각도를 설정합니다.
        Serial.println(posDegrees);
        delay(1);
    }
}
