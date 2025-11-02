#include <Arduino.h>
#include <M5Unified.h>
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include "MyLittleFS.h"

#define MSG_BUFFER_SIZE (8)
char msg[MSG_BUFFER_SIZE] = "NOT";
char lastMsg[MSG_BUFFER_SIZE];
char receivedMsg[MSG_BUFFER_SIZE];

const char* mqtt_server = "mqtt.m5stack.com";
const char* switch_topic = "M5Stack/ATOM/PC_SWITCH/SWITCH";
const char* state_topic = "M5Stack/ATOM/PC_SWITCH/STATE";

const int neopixelPin = GPIO_NUM_27;
const int RelayPin    = GPIO_NUM_32; // ATOM LITE  GND 5V G26 G32
const int statePin    = GPIO_NUM_33; // Input Pin to read state

const int neopixelCount = 1;
int pcState = 0;

unsigned long flow = 0;
bool whileCallback = false;

MyLittleFS* mySPIFFS = new MyLittleFS();
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel pixels(neopixelCount, neopixelPin, NEO_GRB + NEO_KHZ800);


void init_WiFi();
void powerToggle();
void readState();
void beginWiFiManager();
void reConnect();
void callback(char* topic, byte* payload, unsigned int length);
void setColor(int r, int g, int b, int index = 0, int brightness = 50);


void setup() {
  M5.begin();
  Serial.begin(115200);
  Serial.println("M5Unified initialized.");

  M5.BtnA.setHoldThresh(1000);
  M5.BtnA.setDebounceThresh(50);
  
  pinMode(RelayPin, OUTPUT);
  pixels.begin();
  setColor(255, 0, 0, 0, 10); // Turn off initially
  
  mySPIFFS->InitLitteFS();
  delay(100);

  mySPIFFS->listDir(LittleFS, "/", 0);
  Serial.println("Connecting to WiFi...");

  if(mySPIFFS->loadConfig(LittleFS))
  {
    init_WiFi();
  }
  else
  {
    //const char* ssid        = "";
    //const char* password    = "";
    //WiFi.begin(ssid, password);
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }
  delay(500);

  beginWiFiManager();
  client.setServer(mqtt_server, 1883);  // Sets the server details. 
  client.setCallback(callback);  // Sets the message callback function.  
  setColor(0, 0, 255, 0, 50); // Green color to indicate ready
}

void loop() {
  if (!client.connected()) {
        reConnect();
  }
  client.loop();
  M5.update();
  int btnState = M5.BtnA.wasHold()     ? 1
               : M5.BtnA.wasClicked()  ? 2
               : M5.BtnA.isPressed()   ? 3
               : M5.BtnA.wasReleased() ? 4
               : M5.BtnA.wasDecideClickCount() ? 5
               : 0;

  switch (btnState)
  {
  case 1:
    Serial.println("Button A was held.");
    break;
  case 2:
    powerToggle();
    Serial.println("Power Toggle.");
    break;
  case 3:
    break;
  case 4:
    Serial.println("Button A was wasReleased."); // Released after holding button
    break;
  case 5:
    break;
  default:
    break;
  }
  readState();
}

void init_WiFi()
{
    Serial.println(mySPIFFS->ssid);
    Serial.println(mySPIFFS->pass);

    WiFi.mode(WIFI_STA); // 
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    //delay(3000);
    Serial.printf("IP : %s\r\n", WiFi.localIP().toString().c_str());
    Serial.println("Connect to Flash Memory");

}


void powerToggle()
{
  digitalWrite(RelayPin, HIGH);
  delay(50); 
  digitalWrite(RelayPin, LOW);
}

void readState()
{
  static unsigned long lastStateTime = 0;
  if (millis() - lastStateTime >= 1000)
  {
    lastStateTime = millis();
    pcState = analogRead(statePin);
    Serial.printf("PC STATE : %d\r\n", pcState);
    if(pcState >= 4000)
    {
      client.publish(state_topic, "ON");
    }
    else
    {
      client.publish(state_topic, "OFF");
    }
  }
}

void setColor(int r, int g, int b, int index, int brightness)
{
  pixels.setBrightness(brightness);
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}


void beginWiFiManager()
{
  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      if (millis() - connectionLastTime > 10000)
      {
        Serial.println("Start WiFiManager => 192.168.4.1");
        setColor(0, 255, 0, 0, 50); 
        wifiManager.resetSettings();
        bool wmRes = wifiManager.autoConnect("ESP32-ATOM-RELAY");
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.printf("\nSuccess\n");
          Serial.printf("%s  | %s \r\n", wifiManager.getWiFiSSID().c_str(), wifiManager.getWiFiPass().c_str() );
          mySPIFFS->saveConfig(LittleFS, wifiManager.getWiFiSSID().c_str(), wifiManager.getWiFiPass().c_str());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }

  }
}

void reConnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID.  创建一个随机的客户端ID
        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect.  尝试重新连接
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement to the topic.
            client.publish(state_topic, "hello world"); // publish To Connect
            
            client.subscribe(switch_topic);    // Subsrcibe
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
      powerToggle();
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