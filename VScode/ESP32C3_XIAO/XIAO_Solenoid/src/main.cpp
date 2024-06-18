#include <Arduino.h>
#include <HardwareSerial.h>
#include "neopixel.h"

#define HWSERIAL Serial1

const int relay_1_Pin = 21;
const int relay_2_Pin = 20;
const int led_Pin = 4;      // GPIO4
const int button_pin = 5;

bool relayState = false;
uint8_t led_Value = 0;

int flow = 0;
uint64_t flowTime = 0;

MyNeopixel* myNeopixel = new MyNeopixel();

void setRelay1();
void setRelay2();

void setup() {
  Serial.begin(115200);
  HWSERIAL.begin(115200, SERIAL_8E1, 10, 9); // RS485
  pinMode(relay_1_Pin, OUTPUT);
  pinMode(relay_2_Pin, OUTPUT);
  pinMode(button_pin, INPUT);   // Hardware Pull-Up

  //digitalWrite(relay_1_Pin, LOW);
  //digitalWrite(relay_2_Pin, LOW);

  ledcAttachPin(led_Pin, 0);
  ledcSetup(0, 5000, 8);
  ledcWrite(0, 0);

  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0),10, 1 );
}

void loop() {
  if(millis() - flowTime > 1000)
  {
    flowTime = millis();
    flow++;
    Serial.printf("%d\r\n", flow);
    HWSERIAL.printf("%d\r\n", flow);
  }
  
  if(!digitalRead(button_pin))
  {
    if(relayState)
    {
      setRelay1();
    } 
    else
    {
      setRelay2();
    }
    relayState = !relayState;
    while (!digitalRead(button_pin))
    {
      Serial.printf("Button Clicking\r\n");
      delay(100);
    }
    delay(50);
  }
 
  int ch = Serial.read();
  
  if(ch != -1)
  {
    // Serial.printf("READ : %d\r\n", ch);
    switch (ch)
    {
    case 'a':
      
      if (led_Value >= 255)
      {
        led_Value = 255;
      }
      else
      {
        led_Value++;
      }
      Serial.printf("Value : %d\r\n", led_Value);
      HWSERIAL.printf("LED : %d\r\n", led_Value);
      ledcWrite(0, led_Value);
      delay(10);
      break;
    case 'b':
      
      if (led_Value <= 0)
      {
        led_Value = 0;
      }
      else
      {
        led_Value--;
      }
      Serial.printf("Value : %d\r\n", led_Value);
      HWSERIAL.printf("LED : %d\r\n", led_Value);
      ledcWrite(0, led_Value);
      delay(10);
      break;  
    case 'c':
      setRelay1();
      break;

    case 'd':
      setRelay2();
      break;

    default:      
      break;
    }
 }

  // for (int i = 0; i < 255; i++)
  // {
  //   ledcWrite(0, i);
  //   delay(5);
  // }
  // for (int i = 255; i >0; i--)
  // {
  //   ledcWrite(0, i);
  //   delay(5);
  // }
  
  
}


void setRelay1()
{
  digitalWrite(relay_1_Pin, LOW);
  delay(100);
  digitalWrite(relay_1_Pin, HIGH);
  delay(100);
  digitalWrite(relay_1_Pin, LOW);
  delay(100);
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0),10, 1 );
  Serial.printf("Push Solenoid\r\n");
}

void setRelay2()
{
  digitalWrite(relay_2_Pin, LOW);
  delay(100);
  digitalWrite(relay_2_Pin, HIGH);
  delay(100);
  digitalWrite(relay_2_Pin, LOW);
  delay(100);
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255),10, 1 );
  Serial.printf("Release Solenoid\r\n");
}