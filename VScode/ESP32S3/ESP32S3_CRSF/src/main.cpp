#include <Arduino.h>
#include <FastLED.h>
#include <HardwareSerial.h>

#define PIN_BUTTON 0
#define PIN_LED    21
#define NUM_LEDS   1

CRGB leds[NUM_LEDS];
uint8_t led_ih             = 0;
uint8_t led_status         = 0;
String led_status_string[] = {"Rainbow", "Red", "Green", "Blue"};

uint64_t g_count = 0;
uint64_t lastTime = 0;
uint64_t ledTime = 0;

HardwareSerial mySerial(1);

void setup() {
  USBSerial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 1, 3);

  pinMode(PIN_BUTTON, INPUT);
  FastLED.addLeds<WS2812, PIN_LED, GRB>(leds, NUM_LEDS);

  USBSerial.println("StampS3 Servo Start");

}
char text;
void loop() {
  // if(millis() - lastTime > 1000)
  // {
  //   USBSerial.printf("Flow : %d\r\n", g_count++);
  //   mySerial.printf("MyUART : %d\r\n", g_count);
  //   lastTime = millis();
  // }

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
      USBSerial.printf("Press 1\r\n");  
      mySerial.printf("Press 1\r\n");
      break;
    case '2':
      USBSerial.printf("Press 2\r\n");
      mySerial.printf("Press 2\r\n");  

      break;
    }

  }
  text = mySerial.read();
  if(text == -1)
  { 
    USBSerial.printf("DATA : %c\r\n", mySerial.read());
  }
  else
  {    
    if(millis() - lastTime > 1000)
    {
      USBSerial.printf("DATA : Not yet\r\n");
      lastTime = millis();
    }
  }
  


  
}

