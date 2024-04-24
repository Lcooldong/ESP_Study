#include "mainFunc.h"


#ifdef ESP32Dev
const char* apName = "AP_ESP32Dev";
#endif

#ifdef LOLIN32
const char* apName = "AP_LOLIN32";
IPAddress ip (192, 168, 1, 48);
IPAddress gateway (192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
#endif

//#define MASTER
//#define ESPNOW


void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_SENSOR_PIN, INPUT);

  initOLED(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 0);
  u8x8.printf("Start ESP32");


  myNeopixel->InitNeopixel();
  pinMode(LED, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 1);

  setUpWiFi();
  setupOTA();

#ifdef ESPNOW
  setupESPNOW();

#ifdef MASTER
  setupESPNOWPair();
#endif

#endif

  delay(1000);
}




int count = 0;
unsigned long now = 0;
unsigned long ledToggleTimer = 0;

void loop() {
  ArduinoOTA.handle();
  ElegantOTA.loop();

  if( ledState != 0)
  {
    now = millis();  // update
  }
  // 100ms
  if (millis() - touchLastTime > TOUCH_SENSOR_INTERVAL)
  {
    ledState = digitalRead(TOUCH_SENSOR_PIN);
    Serial.printf("Value = %d | Count : %d\r\n", ledState, count );
    
    touchLastTime = millis();
  }


  if (now - ledToggleTimer >= 1000)
  {


  }

  if(ledState == 1)
  {
    if( ledState != digitalRead(TOUCH_SENSOR_PIN))
    {
      Serial.println("1");
    }
    else
    {
      Serial.println("2");
    }
    // int triggerTime = millis();   // 시작 시간
    // int currentTime = 0;

    
    // digitalWrite(BUILTIN_LED, HIGH);
    // if (currentTime - triggerTime >= 1000) // 멈춰버림
    // {
    //   currentTime = millis(); // 현재 시간
    // }
    // else
    // {
    //   ledToggle = !ledToggle;
    //   // Serial.printf("Toggle : %d\r\n", ledToggle);
    //   digitalWrite(BUILTIN_LED, LOW);
    // }
  }
  
 

#ifdef MASTER
  if(Serial.available())
  {
    Serial.readBytes((char*)&serialData, sizeof(serialData));
    if(serialData.status == 1)
    {

    }
    delay(1);
  }
  sendData();
#endif

  resetBoardValue();
  u8x8.refreshDisplay();
}

