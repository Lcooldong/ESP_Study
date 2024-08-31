#include <Arduino.h>

#define LED_CHANNEL 0
#define LED_FREQUENCY 50000
#define LED_RESOLUTION 8

const int BRIGHTNESS_PIN = D2;
const int LED_PIN = D3;

uint8_t ledValue = 0;
uint64_t brightnessLastTime = 0;


void setup() {
  Serial.begin(115200);

  pinMode(BRIGHTNESS_PIN, INPUT);
  
  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
  ledcAttachPin(LED_PIN , LED_CHANNEL);


}

void loop() {

  if(millis() - brightnessLastTime > 100)
  {
    int value = analogRead(BRIGHTNESS_PIN);
    uint8_t mapValue = map(value, 0, 4095, 0, 255);
    Serial.printf(">Value:%d\r\n", value);
    Serial.printf(">MapValue:%d\r\n", mapValue);
    ledcWrite(LED_CHANNEL, mapValue);
    brightnessLastTime = millis();
  }



  if(Serial.available())
  {
    int c = Serial.read();

    switch (c)
    {
    case 'u':
      if(ledValue >=255)
      {
        ledValue = 255;
      }
      else
      {
        ledValue++;
      }
      break;
    case 'd':
      if(ledValue <= 0)
      {
        ledValue = 0;
      }
      else
      {
        ledValue--;
      }
      break;
    }
    
    if(c != -1)
    {
      Serial.printf("Value : %d\r\n", ledValue);
    }
    ledcWrite(LED_CHANNEL, ledValue);

  }
}

