#include "mainFunc.h"


#ifdef D1_MINI
const char* apName = "AP_D1_MINI";
#endif

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
void loop() {
  ArduinoOTA.handle();
  ElegantOTA.loop();

  
  if (millis() - hallLastTime > HALL_SENSOR_INTERVAL)
  {
    int value = analogRead(HALL_SENSOR_PIN);
    Serial.printf("Value = %d | Count : %d\r\n", value, count );
    
    hallLastTime = millis();
    


    if(value < HALL_SENSOR_CUTOFF)
    {
      if(count >= 10)
      {
        count = 10;
      }
      else
      {
        ++count;
      }
      
    }
    else
    {
      if(count <= 0)
      {
        count = 0;            
      }
      else
      {
        --count;
      }     
    }  
  }



  if(count == 10)
  {
    digitalWrite(LED, HIGH);
    myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 100), 150, 1);
    WebSerial.println("Light On\r\n");
    u8x8.setCursor(0, 4);
    u8x8.printf("Light On\r\n");
  }
  else if (count == 0)
  {
    digitalWrite(LED, LOW);
    myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 0), 0, 1);
    WebSerial.println("Light Off\r\n");
    u8x8.setCursor(0, 4);
    u8x8.printf("Light Off\r\n");
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

