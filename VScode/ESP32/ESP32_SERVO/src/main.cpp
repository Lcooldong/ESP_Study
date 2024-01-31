#include <Arduino.h>
//#include <Servo.h>
#define ESP32_SERVO

#ifdef ESP32_SERVO
#include <ESP32Servo.h>
#endif



//#define FULL_DEGREE

#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>


//#define LIMIT_SENSOR_PIN 4
#define SERVO_PIN 18
//#define BTN_PIN 39

Servo myservo;

int pos = 0;

void setup() {
  Serial.begin(115200);
//  pinMode(LIMIT_SENSOR_PIN, INPUT_PULLUP);
//  pinMode(BTN_PIN, INPUT_PULLUP);
//  myservo.attach(SERVO_PIN, -1, 0, 180, 1000, 2000, 50);
#ifdef ESP32_SERVO
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN);
 // myservo.attach(SERVO_PIN, 500, 2400);
  myservo.write(0);
#endif

}

void loop() {

  if(Serial.available()){
    char text = Serial.read();
    switch (text)
    {
    case 'u':
      pos += 1;
      Serial.println(pos);
      myservo.write(pos);
      delay(3);
      break;
    case 'd':
      pos -= 1;
      Serial.println(pos);
      myservo.write(pos);
      delay(3);
      break;
    
    default:
      
      
      break;
    }
  }
  // for(pos = 0; pos < 180; pos += 1) { // goes from 0 degrees to 180 degrees, 1 degree steps
  //     myservo.write(pos);              // tell servo to go to position in variable 'pos'
  //     Serial.printf("POS : %d\r\n", pos);
  //     delay(15);                       // waits 15ms for the servo to reach the position
  //   }
  //   delay(500);

  //   for(pos = 180; pos>=1; pos-=1) {   // goes from 180 degrees to 0 degrees
  //     myservo.write(pos);              // tell servo to go to position in variable 'pos'
  //     Serial.printf("POS : %d\r\n", pos);
  //     delay(15);                       // waits 15ms for the servo to reach the position
  //   }
    
  //   delay(500);


#ifdef FULL_DEGREE
  if(digitalRead(BTN_PIN))
  {
    myservo.write(90);
    delay(100);
  }
  else
  {
    myservo.write(0);
    delay(100);
  }
  

  if(!digitalRead(LIMIT_SENSOR_PIN)) // Pressed -> 0,  Release -> 1
  {
    myservo.write(90);  // Stop
  }
  else
  {
    myservo.write(180); // ccw
    // myservo.write(0);   // cw 
  }
 // 0 ~ 1499  CW, 1501 ~ 2500 CCW, 1500 정지 1500에 가까울 수록 느림 (좌우로 1470 ~ 1530 정도는 멈춤)
  // for (int i = 1500; i < 2500; i++)
  // {
  //   myservo.writeMicroseconds(i);
  //   Serial.printf("Pulse Width :  %d\n", i);
  //   delay(100);
    
  // }
#endif


// for (pos = 0; pos < 120; pos++)
// {
//   myservo.write(pos);
//   Serial.printf("Pos : %d\n", pos);
//   delay(10);
// }

// for (pos = 119; pos >= 0; pos--)
// {
//   myservo.write(pos);
//   Serial.printf("Pos : %d\n", pos);
//   delay(10);
// }


 
  
}

