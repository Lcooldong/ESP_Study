#include <Arduino.h>
#include <FastLED.h>
#include <ESP32Servo.h>

#define PIN_BUTTON 0
#define PIN_LED    21
#define NUM_LEDS   1
#define SERVO_PIN  1
#define M7790TH_HIGH 2100
#define M7790TH_LOW   900

CRGB leds[NUM_LEDS];
uint8_t led_ih             = 0;
uint8_t led_status         = 0;
String led_status_string[] = {"Rainbow", "Red", "Green", "Blue"};

uint64_t g_count = 0;
uint64_t lastTime = 0;
uint64_t ledTime = 0;

Servo myServo;
int myServoPos = 0;
const int zeroMicroSec = 1500;
uint16_t posMicroSec = zeroMicroSec;


void setup() {
  USBSerial.begin(115200);
  pinMode(PIN_BUTTON, INPUT);
  FastLED.addLeds<WS2812, PIN_LED, GRB>(leds, NUM_LEDS);

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, M7790TH_LOW, M7790TH_HIGH);


  if(myServo.attached())
  {
    USBSerial.println("Servo Attached!!");
  }



  USBSerial.println("StampS3 Servo Start");
  myServo.writeMicroseconds(zeroMicroSec);
}

void loop() {
  if(millis() - lastTime > 1000)
  {
    USBSerial.printf("Flow : %d\r\n", g_count++);

    lastTime = millis();
  }

  if(millis() - ledTime > 10)
  {
    leds[0] = CHSV(led_ih, 255, 255);
    FastLED.show();
    led_ih++;
    ledTime = millis();
  }

  if(USBSerial.available())
  {
    char ch = USBSerial.read();
    
    switch (ch)
    {
    case '1':
      // myServoPos++;
      // myServo.write(myServoPos);
      if(posMicroSec <= M7790TH_HIGH)
      {
        posMicroSec += 1;
      }
      else
      {
        posMicroSec = M7790TH_HIGH;
      }
      myServo.writeMicroseconds(posMicroSec);
      USBSerial.printf("POS : %d\r\n", posMicroSec);
      break;
    case '2':
      // myServoPos--;
      if(posMicroSec >= M7790TH_LOW)
      {
        posMicroSec -= 1;
      }
      else
      {
        posMicroSec = M7790TH_LOW;
      }
      // myServo.write(myServoPos);
      myServo.writeMicroseconds(posMicroSec);
      USBSerial.printf("POS : %d\r\n", posMicroSec);
      break;
    }

  }

  
}

