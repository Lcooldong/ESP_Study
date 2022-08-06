#include <Arduino.h>
#include "esp_camera.h"
#include <SD.h>
#include <EEPROM.h>
#include "FS.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <WiFi.h>
#include "time.h"

#include "ESP_Mail_Client.h"

#define USE_INCREMENTAL_FILE_NUMBERING

// #define USE_TIMESTAMP
// #define SEND_EMAIL

// #define TRIGGER_MODE
#define TIMED_MODE

#define WIFI_SSID "SK_WiFiGIGA9687"
#define WIFI_PASSWORD "1712042694"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 443
#define AUTHOR_EMAIL "alarmrobotpolytech@gmail.com"
#define AUTHOR_PASSWORD ""

const byte ledPin = GPIO_NUM_33;
const byte flashPin = GPIO_NUM_4;
const byte triggerPin = GPIO_NUM_13;
const byte flashPower = 1;
#ifdef TIMED_MODE
  const int timeLapseInterval = 30;
#endif
const int startupDelayMillis = 3000;





void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}