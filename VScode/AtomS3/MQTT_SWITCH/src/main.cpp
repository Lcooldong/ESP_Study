#define RELAY_PIN 8
#define FAN_ON_PIN 7
#define RESTART_RELAY_PIN  
#define WIFI_CONNECTION_INTERVAL 10000

#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include "ESPAsyncWiFiManager.h"

#include "ImageViewer.hpp"
#include "MyLittleFS.h"

MyLittleFS* mySPIFFS = new MyLittleFS();
AsyncWebServer server(80);
DNSServer dns;

ImageViewer viewer;

WiFiClient espClient;
PubSubClient client(espClient);


const char* mqtt_server = "mqtt.m5stack.com";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
char receivedMsg[MSG_BUFFER_SIZE];
int value = 0;
bool pcState = true;

void forever();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();
void powerOnOff();
void shutdown();

bool btnToggle = false;



void setup() {

 // myLittleFS->InitSPIFFS();
  
  AsyncWiFiManager wifiManager(&server,&dns);
  USBSerial.begin(115200);
  USBSerial.flush();
  delay(1000);

  pinMode(RELAY_PIN, OUTPUT);
  //pinMode(FAN_ON_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, LOW);
  // SPIFFS 적용됨
  if (!viewer.begin()) {
        forever();
  }
  
//  myLittleFS->listDir(SPIFFS, "/", 0);
  USBSerial.println("Connecting to WiFi...");

  if(mySPIFFS->loadConfig(SPIFFS))
  {
    USBSerial.println(mySPIFFS->ssid);
    USBSerial.println(mySPIFFS->pass);
    M5.update();

    WiFi.mode(WIFI_STA);
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    USBSerial.println("Connect to Flash Memory");
    M5.update();
  }
  else
  {
    //const char* ssid        = "Cooldong";
    //const char* password    = "8ec4hkx000";
    //WiFi.begin(ssid, password);
    USBSerial.println("Saved file doesn't exist => Move to WiFiManager");
    M5.update();
  }

  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      USBSerial.print(".");
      M5.update();

      if (millis() - connectionLastTime > WIFI_CONNECTION_INTERVAL)
      {
        USBSerial.println("Start WiFiManager => 192.168.4.1");
        M5.update();
        wifiManager.resetSettings();
        bool wmRes = wifiManager.autoConnect("PC_SWITCH");
        if(!wmRes)
        {
          USBSerial.println("Failed to connect");
        }
        else
        {
          USBSerial.printf("\nSuccess\n");
          M5.update();
          M5.Lcd.clear();
          M5.Lcd.println("Success");
          
          mySPIFFS->saveConfig(SPIFFS, wifiManager.getConfiguredSTASSID(), wifiManager.getConfiguredSTAPassword());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }

  }
  
  
  
  USBSerial.printf("\n%s\r\n", "Start MQTT Setup");
  M5.update();

  client.setServer(mqtt_server, 1883);  // Sets the server details. 
  client.setCallback(callback);  // Sets the message callback function.  

  
  // M5.IMU.begin();
  // USBSerial.printf("0x%02x\n", M5.IMU.whoAmI());




}

float ax, ay, az, gx, gy, gz, t;
uint8_t num[3] = {0x00, 0x00, 0x00};

int count = 0;
unsigned long lastTime = 0;
unsigned int interval = 100;
unsigned long restartTime = 4000000000;
unsigned long beforeShutdown = 0;
unsigned int shutdownTimeOut = 10000;

void loop() {

  if (!client.connected()) {
        reConnect();
  }
  client.loop();

  // IMAGE
  if (millis() - lastTime > interval)
  {
    viewer.update();
    lastTime = millis();
    int analogValue = analogRead(FAN_ON_PIN);
    //M5.Lcd.print(pcState);
    //M5.update();
    //M5.Lcd.clear();
    
    if(analogValue > 4000)
    {
      snprintf(msg, MSG_BUFFER_SIZE, "ON");
      pcState = true;
    }
    else
    {
      snprintf(msg, MSG_BUFFER_SIZE, "OFF");
      pcState = false;
    }
    client.publish("M5Stack/LCD/AtomS3/PC_STATE", msg);
    if(lastTime > restartTime)
    {
      ESP.restart();
    }
  }

  if(M5.BtnA.pressedFor(2000))
  {
    shutdown();
    
  }
  else if(M5.BtnA.wasClicked())
  {
    powerOnOff();
    USBSerial.println("Button Clicked");
  }
  

  // USBSerial.println(analogRead(FAN_ON_PIN));
  // delay(500);
  // if(M5.BtnA.isPressed())
  // {    
  //   USBSerial.printf("Count : %d\r\n", count++);

  //   snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", count);  // Format to the specified string and store it in MSG.

  //   USBSerial.print("Publish message: ");
  //   USBSerial.println(msg);

  //   // Test -> Android
  //   //  client.publish("M5Stack/LCD/AtomS3/PC_SWITCH", msg);  // Publishes a message to the specified    
    
  //   //powerOnOff();
  //   M5.update();
  //   delay(100);
  // }
  // if(count > 20)
  // {
  //   shutdown();
  //   count = 0;
  // }
  // else if (M5.BtnA.wasReleased())
  // {
  //   count = 0;
  // }
  // else if (M5.BtnA.isReleased())
  // {
  //   USBSerial.println("PowerOff");
  //   delay(1000);
  //   //powerOnOff();
  // }

  // unsigned long now = millis();  // Obtain the host startup duration.  获取主机开机时长
  //   if (now - lastMsg > 2000) {
  //       lastMsg = now;
  //       ++value;
  //       snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);  // Format to the specified string and store it in MSG.

  //       USBSerial.print("Publish message: ");
  //       USBSerial.println(msg);
  //       client.publish("M5Stack", msg);  // Publishes a message to the specified
                                  
  //   }

}





void forever(void) {
    while (true) {
        delay(1);
    }    
}


void callback(char* topic, byte* payload, unsigned int length) {
    USBSerial.print("Message arrived [");
    USBSerial.print(topic);
    USBSerial.print("] : ");
    USBSerial.print("(");
    USBSerial.print(length);
    USBSerial.print(") -> ");
    memcpy(&receivedMsg, payload, length);

    USBSerial.printf("Data : %d =>> %s", pcState, receivedMsg);
    USBSerial.println();
    

    if(!strcmp(receivedMsg, "ON"))
    {
      if (!pcState)
      {
        USBSerial.println("turn on pc");
        powerOnOff();
      }
      
    }
    else if (!strcmp(receivedMsg, "OFF"))
    {
      if (pcState)
      {
        USBSerial.println("turn off pc");
        powerOnOff();
      }
    }

    memset(&receivedMsg, 0x00, length);

    M5.update();

  //  byte* p = (byte*)malloc(length);
   // memcpy(p, payload, length);

   // free(p);

    // for (int i = 0; i < length; i++) {
    //     USBSerial.print((char)payload[i]);
    // }
    

}

void reConnect() {
    while (!client.connected()) {
        USBSerial.print("Attempting MQTT connection...");
        // Create a random client ID.  创建一个随机的客户端ID
        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect.  尝试重新连接
        if (client.connect(clientId.c_str())) {
            USBSerial.println("connected");
            // Once connected, publish an announcement to the topic.
            client.publish("M5Stack/LCD/AtomS3/PC_SWITCH", "hello world"); // publish To Connect

            client.subscribe("M5Stack/LCD/AtomS3/PC_SWITCH");    // Subsrcibe
        } else {
            USBSerial.print("failed, rc=");
            USBSerial.print(client.state());
            USBSerial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}


void powerOnOff()
{
    digitalWrite(RELAY_PIN, LOW);
    delay(100);
    digitalWrite(RELAY_PIN, HIGH);
    delay(100);
    digitalWrite(RELAY_PIN, LOW);
}

void shutdown()
{
  int fanValue = analogRead(FAN_ON_PIN);
  
  while( fanValue > 4000)
  {
    digitalWrite(RELAY_PIN, HIGH);
    delay(100);

    if(millis() - beforeShutdown > shutdownTimeOut)
    {
      beforeShutdown = millis();
      USBSerial.println("Shutdown TimeOut");
      M5.update();
      break;
    }
  }

  if ( fanValue < 1000)
  {
    USBSerial.println("PC was already turned off | Check Wire Connection");
    M5.update();
  }

  digitalWrite(RELAY_PIN, LOW);

}