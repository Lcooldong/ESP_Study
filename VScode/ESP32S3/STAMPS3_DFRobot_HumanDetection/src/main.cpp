#include <Arduino.h>
#include <FastLED.h>
#include <DFRobot_HumanDetection.h>
#include <HardwareSerial.h>

// NOT Good Sensor

// https://www.dfrobot.com/forum/topic/335849?srsltid=AfmBOorVCGT2ChiANDdHEssGN3lEy8OnkVa_fsThuOzxVQ-pyX2iLBx3
// https://www.dfrobot.com/forum/topic/339554?srsltid=AfmBOoq5rytzbP9ov5pxQwCLyUVkog0Y72FoEIka6OCx2rsvfv9sEDmZ

const int RX1_PIN    = 1;
const int TX1_PIN    = 3;
const int PIN_BUTTON = 0;
const int PIN_LED    = 21;
const int NUM_LEDS   = 1;

uint32_t lastMillis = 0;
int count = 0;

CRGB leds[NUM_LEDS];
DFRobot_HumanDetection hu(&Serial1);



void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  pinMode(PIN_BUTTON, INPUT);
  FastLED.addLeds<WS2812, PIN_LED, GRB>(leds, NUM_LEDS);

  leds[0] = CRGB(255, 0, 0);
  // leds[0] = CHSV(0, 100, 255);
  //leds[0] = CRGB::Blue;
  FastLED.show();
  delay(100);


  Serial.println("Start initialization");
  while (hu.begin() != 0)
  {
    Serial.println("init Error!!");
    delay(1000);
  }
  Serial.println("Initialization successful");

  Serial.println("Start switching work mode");
  //while (hu.configWorkMode(hu.eFallingMode) != 0) {
  while (hu.configWorkMode(hu.eSleepMode) != 0) {
    Serial.println("error!!!");
    delay(1000);
  }
  Serial.println("Work mode switch successful");

  Serial.print("Current work mode:");
  switch (hu.getWorkMode()) {
    case 1:
      Serial.println("Fall detection mode");
      break;
    case 2:
      Serial.println("Sleep detection mode");
      break;
    default:
      Serial.println("Read error");
  }


  // 0 : OFF, 1 : ON (present)
  hu.configLEDLight(hu.eHPLed, 1);  // Set HP LED switch, it will not light up even if the sensor detects a person when set to 0.
  hu.configLEDLight(hu.eFALLLed, 1);
  //hu.dmUnmannedTime
  hu.dmInstallHeight(100);
  
  hu.sensorRet();                   // Module reset, must perform sensorRet after setting data, otherwise the sensor may not be usable

  Serial.print("HP LED status:");
  switch (hu.getLEDLightState(hu.eHPLed)) {
    case 0:
      Serial.println("Off");
      break;
    case 1:
      Serial.println("On");
      break;
    default:
      Serial.println("Read error");
  }
  leds[0] = CRGB(0, 255, 0);
  FastLED.show();
  
}

void loop() {
  
  uint32_t currentMillis = millis();
  if(currentMillis - lastMillis >= 1500)
  {
    count++;
    //Serial.printf("[%d]\r\n", count);

    Serial.print("Existing information:");
    //Serial.printf("[%d]-> %d\r\n", count, hu.smHumanData(hu.eHumanPresence));
    Serial.printf("[%d]-> %d | %d cm | %d\r\n", count, hu.smHumanData(hu.eHumanPresence), hu.smHumanData(hu.eHumanDistance), hu.smHumanData(hu.eHumanMovingRange));
    
    // switch (hu.smHumanData(hu.eHumanPresence)) 
    // {
    //   case 0:
    //     Serial.println("No one is present");
    //     break;
    //   case 1:
    //     Serial.println("Someone is present");
    //     break;
    //   default:
    //     Serial.println("Read error");
    // }

    // Serial.print("Motion information:");
    // switch (hu.smHumanData(hu.eHumanMovement)) 
    // {
    //   case 0:
    //     Serial.println("None");
    //     break;
    //   case 1:
    //     Serial.println("Still");
    //     break;
    //   case 2:
    //     Serial.println("Active");
    //     break;
    //   default:
    //     Serial.println("Read error");
    // }

    // Serial.printf("Body movement parameters:%d\n", hu.smHumanData(hu.eHumanMovingRange));
    //Serial.printf("Respiration rate:%d\n", hu.getBreatheValue()); // NOT WORKING
    //Serial.printf("Heart rate:%d\n", hu.getHeartRate());          // NOT WORKING
    Serial.println();



    lastMillis = currentMillis;
  }

  
}
