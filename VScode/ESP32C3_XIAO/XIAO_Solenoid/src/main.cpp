#include <Arduino.h>
#include <HardwareSerial.h>

#define HWSERIAL Serial1

const int relay_1_Pin = 21;
const int relay_2_Pin = 20;
const int led_Pin = 4;      // GPIO4

uint8_t led_Value = 0;

int flow = 0;
uint64_t flowTime = 0;

void setup() {
  Serial.begin(115200);
  HWSERIAL.begin(115200, SERIAL_8E1, 10, 9); // RS485
  pinMode(relay_1_Pin, OUTPUT);
  pinMode(relay_2_Pin, OUTPUT);

  // analogWrite(1, 255);
  ledcAttachPin(led_Pin, 0);
  ledcSetup(0, 5000, 8);
  
  digitalWrite(relay_1_Pin, LOW);
  digitalWrite(relay_2_Pin, LOW);
  ledcWrite(0, 0);
}

void loop() {
  if(millis() - flowTime > 1000)
  {
    flowTime = millis();
    flow++;
    Serial.printf("%d\r\n", flow);
    HWSERIAL.printf("%d\r\n", flow);  
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
      digitalWrite(relay_1_Pin, LOW);
      delay(50);
      digitalWrite(relay_1_Pin, HIGH);
      delay(50);
      digitalWrite(relay_1_Pin, LOW);
      delay(50);
      Serial.printf("Push Solenoid\r\n");
      break;

    case 'd':
      digitalWrite(relay_2_Pin, LOW);
      delay(50);
      digitalWrite(relay_2_Pin, HIGH);
      delay(50);
      digitalWrite(relay_2_Pin, LOW);
      delay(50);
      Serial.printf("Release Solenoid\r\n");
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

