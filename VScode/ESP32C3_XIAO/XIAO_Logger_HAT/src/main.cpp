#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include <BH1750.h>
#include <PCF8563.h>
#include <Adafruit_SHT4X.h>

#include <PubSubClient.h>
#include "MyWiFi.h"

// #include <NTPClient.h>
// #include <WiFiUdp.h>

#include <ESPNtpClient.h>

#define SHOW_TIME_PERIOD 1000
#define NTP_TIMEOUT      1500
#define ONBOARDLED       2
#define BATTERY_PIN      D1

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
sensors_event_t humidity, temp;
BH1750 lightMeter(0x23);
PCF8563 pcf;
WiFiClient espClient;
PubSubClient client(espClient);



const PROGMEM char* ntpServer = "pool.ntp.org";
bool wifiFirstConnected = false;
boolean syncEventTriggered = false; // True if a time even has been triggered
NTPEvent_t ntpEvent; // Last triggered event
double offset;
double timedelay;


String formattedDate;
String dayStamp;
String timeStamp;
unsigned long flow = 0;
bool whileCallback = false;

#define MSG_BUFFER_SIZE (8)
char msg[MSG_BUFFER_SIZE] = "NOT";
char lastMsg[MSG_BUFFER_SIZE];
char receivedMsg[MSG_BUFFER_SIZE];

const char* mqtt_server  = "mqtt.m5stack.com";
const char* led_topic  = "M5Stack/XIAO/LOGGER_HAT/LED";
const char* sht40_topic  = "M5Stack/XIAO/LOGGER_HAT/SHT40";
const char* bh1750_topic = "M5Stack/ATOM/PC_SWITCH/BH1750";
const char* rtc_topic    = "M5Stack/XIAO/LOGGER_HAT/RTC";

bool measureLight();
bool measureSHT40();
void readRTC();
void reConnect();
void callback(char* topic, byte* payload, unsigned int length);
int readBatteryLevel();

void onWifiEvent (system_event_id_t event, system_event_info_t info) {
    Serial.printf ("[WiFi-event] event: %d\n", event);

    switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.printf ("Connected to %s. Asking for IP address.\r\n", info.connected.ssid);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.printf ("Got IP: %s\r\n", IPAddress (info.got_ip.ip_info.ip.addr).toString ().c_str ());
        Serial.printf ("Connected: %s\r\n", WiFi.status () == WL_CONNECTED ? "yes" : "no");
        digitalWrite (ONBOARDLED, LOW); // Turn on LED
        wifiFirstConnected = true;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.printf ("Disconnected from SSID: %s\n", info.disconnected.ssid);
        Serial.printf ("Reason: %d\n", info.disconnected.reason);
        digitalWrite (ONBOARDLED, HIGH); // Turn off LED
        //NTP.stop(); // NTP sync can be disabled to avoid sync errors
        WiFi.reconnect ();
        break;
    default:
        break;
    }
}

void processSyncEvent (NTPEvent_t ntpEvent) {
    switch (ntpEvent.event) {
        case timeSyncd:
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            Serial.printf ("[NTP-event] %s\n", NTP.ntpEvent2str (ntpEvent));
            break;
        default:
            break;
    }
}



void setup() {
  Serial.begin(115200);
  Serial.println("Setup started.");
  Wire.begin();
  beginWiFi("ESP32-XIAO-LOGGER-HAT");


  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  switch (sht4.getPrecision()) {
     case SHT4X_HIGH_PRECISION: 
       Serial.println("High precision");
       break;
     case SHT4X_MED_PRECISION: 
       Serial.println("Med precision");
       break;
     case SHT4X_LOW_PRECISION: 
       Serial.println("Low precision");
       break;
  }

  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER);
  switch (sht4.getHeater()) {
     case SHT4X_NO_HEATER: 
       Serial.println("No heater");
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Serial.println("High heat for 1 second");
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Serial.println("High heat for 0.1 second");
       break;
     case SHT4X_MED_HEATER_1S: 
       Serial.println("Medium heat for 1 second");
       break;
     case SHT4X_MED_HEATER_100MS: 
       Serial.println("Medium heat for 0.1 second");
       break;
     case SHT4X_LOW_HEATER_1S: 
       Serial.println("Low heat for 1 second");
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Serial.println("Low heat for 0.1 second");
       break;
  }

  NTP.setTimeZone (TZ_Asia_Seoul);
  NTP.begin ("pool.ntp.org");
  // NTP.settimeSyncThreshold(1000);
  
  String timeStr = NTP.getTimeDateStringForJS ();
    // Serial.println (NTP.getTimeDateStringForJS ()); // parsing friendly format

  int month = timeStr.substring(0, 2).toInt();
  int day   = timeStr.substring(3, 5).toInt();
  int year  = timeStr.substring(6, 10).toInt();

  // 시간 부분
  int hour   = timeStr.substring(11, 13).toInt();
  int minute = timeStr.substring(14, 16).toInt();
  int second = timeStr.substring(17, 19).toInt();

  pcf.init();//initialize the clock

  pcf.stopClock();//stop the clock

  //set time to to 31/3/2018 17:33:0
  pcf.setYear(year);//set year
  pcf.setMonth(month);//set month
  pcf.setDay(day);//set dat
  pcf.setHour(hour);//set hour
  pcf.setMinut(minute);//set minut
  pcf.setSecond(second);//set second

  pcf.startClock();//start the clock

  client.setServer(mqtt_server, 1883);  // Sets the server details. 
  client.setCallback(callback);  // Sets the message callback function.  
  
}

void loop() {
  if (!client.connected()) {
        reConnect();
  }
  client.loop();
  readRTC();

  if(measureLight())
  {
    client.publish(bh1750_topic, "measured");
  }
  if( measureSHT40())
  {
    client.publish(sht40_topic, "measured");
  }

  
}


bool measureLight()
{
  if (lightMeter.measurementReady())
  {
    float lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    return true;
  }
  else
  {
    return false;
  }
}

bool measureSHT40()
{
  static unsigned long lastMeasureTime = 0;
  if (millis() - lastMeasureTime >= 2000)
  {

    lastMeasureTime = millis();
    
    uint32_t timestamp = millis();
    bool sht4State = sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
    timestamp = millis() - timestamp;

    Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

    Serial.print("Read duration (ms): ");
    Serial.println(timestamp);

    return sht4State;
  }
  else
  {
    return false;
  }
}

void readRTC()
{
 static unsigned long lastRTCTime = 0;
  if (millis() - lastRTCTime >= 1000)
  {
    lastRTCTime = millis();

    // timeClient.update();

    // Serial.println(timeClient.getFormattedTime());
    // Serial.println (NTP.getTimeDateStringUs ());
    String timeStr = NTP.getTimeDateStringForJS ();
    // Serial.println (NTP.getTimeDateStringForJS ()); // parsing friendly format

    Time nowTime = pcf.getTime();//get current time
    
  //print current time
    Serial.print(nowTime.day);
    Serial.print("/");
    Serial.print(nowTime.month);
    Serial.print("/");
    Serial.print(nowTime.year);
    Serial.print(" ");
    Serial.print(nowTime.hour);
    Serial.print(":");
    Serial.print(nowTime.minute);
    Serial.print(":");
    Serial.println(nowTime.second);
  }
}


void reConnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");

        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement to the topic.
            client.publish(sht40_topic,  "reconnect"); // publish To Connect
            client.publish(bh1750_topic, "reconnect"); // publish To Connect
            client.publish(rtc_topic,    "reconnect"); // publish To Connect

            client.subscribe(led_topic);    // Subsrcibe
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {

    whileCallback = true;
    flow++;
    Serial.printf("[%05d] Message arrived [", flow);
    Serial.print(topic);
    Serial.print("] : ");
    Serial.print("Length=(");
    Serial.print(length);
    Serial.print(") -> ");
    memcpy( &receivedMsg, payload, length);
    memcpy( &msg, &receivedMsg, MSG_BUFFER_SIZE);
    
    Serial.printf("Data : %s\r\n", msg);
    

    if(!strcmp(receivedMsg, "PRESS"))
    {
      Serial.println("Button PRESS received.");

    }
    
    
    if(!strcmp(receivedMsg, lastMsg))
    {
      if(!strcmp(msg, "ON"))
      {

      }
      else if (!strcmp(msg, "OFF"))
      {
        
      }
    }

    memcpy( &lastMsg, &receivedMsg, MSG_BUFFER_SIZE);
    memset(&receivedMsg, 0x00, length);

    whileCallback = false;
}

int readBatteryLevel()
{
  int sensorValue = analogRead(BATTERY_PIN);
  float voltage = sensorValue * (3.3 / 4095.0) * 2; // ESP32 ADC is 12-bit, so max value is 4095
  return (int)(voltage * 1000); // Return in millivolts
}