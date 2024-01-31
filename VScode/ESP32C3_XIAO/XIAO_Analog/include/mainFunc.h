#ifndef __MAINFUNC_H__
#define __MAINFUNC_H__

#include <Arduino.h>
#include "neopixel.h"
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WebSerial.h>
#include <ESPAsyncWiFiManager.h>
#include <ArduinoOTA.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "MyLittleFS.h"
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//#define SLAVE
//#define XIAO
//#define M5STAMP
//#define LOLIN32
//#define D1_MINI
#define ESP32Dev
//#define FIXED_IP

#ifdef M5STAMP
#define HALL_SENSOR_PIN 7
#define LED             5
// #define HALL_SENSOR_CUTOFF 3000 // 5V
#define HALL_SENSOR_CUTOFF 1900 // 3.3V
#endif

#ifdef XIAO
#define HALL_SENSOR_PIN 4   // GPIO4 = D2 (A2)
#define LED             5   // GPIO5 = D3 (A3)
#define HALL_SENSOR_CUTOFF 1700
#endif

#ifdef LOLIN32
#define HALL_SENSOR_PIN 34   // GPIO4 = D2 (A2)
#define LED             5   // GPIO5 = D3 (A3)
#define HALL_SENSOR_CUTOFF 2400
#endif

#ifdef D1_MINI
#define HALL_SENSOR_PIN 34
#define LED             5   // GPIO5 = D3 (A3)
#define HALL_SENSOR_CUTOFF 2500
#endif

#ifdef ESP32Dev
#define HALL_SENSOR_PIN 34
#define LED             5
#define BUILTIN_LED     2
#define HALL_SENSOR_CUTOFF 2600 // 5V
//#define HALL_SENSOR_CUTOFF 2500 // 3.3V
#endif

#define HALL_SENSOR_INTERVAL 50
#define RESET_DEADLINE 400000000

#define WIFI_CONNECTION_INTERVAL 10000
#define PRINTSCANRESULTS 1
#define DELETEBEFOREPAIR 0

#define SDA_PIN 21
#define SCL_PIN 22

#pragma pack(push, 1)
typedef struct _packet
{ 
    uint8_t STX;
    uint8_t device_id;
    uint8_t status;
    uint8_t RED;  // 데이터 타입 : 1
    uint8_t GREEN;
    uint8_t BLUE;
    uint8_t brightness;
    uint8_t wait;
    uint8_t checksum;  // 체크섬 : 2
    uint8_t ETX;
}PACKET;
#pragma pack(pop)


enum
{
  HEARTBEAT = 0,
  
};

extern PACKET serialData;
extern PACKET incomingReadings;
extern esp_now_peer_info_t slave;
extern uint8_t data;
extern String compareApName;
extern String slaveApName;
extern int sendCompleteFlag; 
extern int32_t channel;


extern uint64_t hallLastTime;
extern unsigned long ota_progress_millis;


extern AsyncWebServer server;
extern DNSServer dns;
extern MyLittleFS* mySPIFFS;
extern MyNeopixel* myNeopixel;
extern U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8;
//extern U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8;


//////////////////////////////////////////
extern const char* apName;

void initOLED(const uint8_t* _font);
int32_t getWiFiChannel(char *ssid);
void resetBoardValue();
void setUpWiFi();
void setupESPNOW();
void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);
void recvMsg(uint8_t *data, size_t len);
void arduinoOTAProgress();
void setupOTA();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void setupESPNOWPair();
void ScanForSlave();
bool manageSlave();
void deletePeer();
void sendData();

#endif