#include <Arduino.h>
#include <FastLED.h>
#include <DFRobot_mmWave_Radar.h>

HardwareSerial mySerial(1);
DFRobot_mmWave_Radar sensor(&mySerial);


const int PIN_BUTTON = 0;
const int PIN_LED    = 21;
const int NUM_LEDS   = 1;

uint32_t lastMillis = 0;
int count = 0;

CRGB leds[NUM_LEDS];


void setup()
{
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 1, 3);//RX,TX
  pinMode(PIN_BUTTON, INPUT);
  FastLED.addLeds<WS2812, PIN_LED, GRB>(leds, NUM_LEDS);

  leds[0] = CRGB(255, 0, 0);
  // leds[0] = CHSV(0, 100, 255);
  //leds[0] = CRGB::Blue;
  FastLED.show();
  delay(100);
  
  sensor.factoryReset();    //Restore to the factory settings 
  sensor.DetRangeCfg(0, 9);    //The detection range is as far as 9m
  sensor.OutputLatency(0, 0);

  leds[0] = CRGB(0, 255, 0);
  FastLED.show();
}

void loop()
{
  int val = sensor.readPresenceDetection();
  
  if(val)
  {
    leds[0] = CRGB(0, 0, 255);
    FastLED.show();
  }
  else
  {
    leds[0] = CRGB(255, 0, 0);
    FastLED.show();
  }
  Serial.println(val);
}