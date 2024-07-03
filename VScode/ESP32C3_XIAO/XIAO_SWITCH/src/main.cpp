#define SERVO_PIN GPIO_NUM_2
#define SERVO_POS_PIN GPIO_NUM_3
#define SWITCH_PIN GPIO_NUM_5
#define WIFI_CONNECTION_INTERVAL 10000
#define SERVO_ON_ANGLE 30
#define SERVO_OFF_ANGLE 0 

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ESP32Servo.h>
#include "Button.h"
#include "MyLittleFS.h"

MyLittleFS* mySPIFFS = new MyLittleFS();

AsyncWebServer server(80);
DNSServer dns;
WiFiClient espClient;
PubSubClient client(espClient);

Servo lightServo;
int targetPos = 0;
bool posChanged = false;
int currentServoPos = 0;
unsigned long servoTime = 0;

Button myBtn(SWITCH_PIN, 0, 10);  // 0 -> HIGH 


const char* mqtt_server = "mqtt.m5stack.com";
const char* light_topic = "M5Stack/LCD/ESP32C3/MYROOM/SERVO_STATE";


#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE] = "NOT";
char lastMsg[MSG_BUFFER_SIZE];
char receivedMsg[MSG_BUFFER_SIZE];


unsigned long flow = 0;
bool lightState = true;
bool whileCallback = false;

unsigned long releaseTime = 0;
unsigned long buttonTime = 0;
bool btnToggle = false;
bool btnPressing = false;
bool btnRelease = false;


int count = 0;
unsigned long lastTime = 0;
unsigned int interval = 1000;
unsigned long restartTime = 4000000000;
unsigned long beforeShutdown = 0;
unsigned int shutdownTimeOut = 10000;

void forever();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();
void localSwitch();
void rotateServo(int _targetPos, uint8_t _delay);




void setup() {

 // myLittleFS->InitSPIFFS();
  Serial.begin(115200);   
  Serial.println("\r\n- Start ESP32C3 -\r\n");
  AsyncWiFiManager wifiManager(&server,&dns);
  
  pinMode(SWITCH_PIN, INPUT);
  // lightServo.attach(SERVO_PIN);
  mySPIFFS->InitLitteFS();

  delay(100);

  mySPIFFS->listDir(LittleFS, "/", 0);
  Serial.println("Connecting to WiFi...");

  if(mySPIFFS->loadConfig(LittleFS))
  {
    Serial.println(mySPIFFS->ssid);
    Serial.println(mySPIFFS->pass);

    WiFi.mode(WIFI_STA); // 
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    Serial.println("Connect to Flash Memory");
  }
  else
  {
    //const char* ssid        = "Cooldong";
    //const char* password    = "8ec4hkx000";
    //WiFi.begin(ssid, password);
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }
  delay(500);

  unsigned long connectionLastTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      if (millis() - connectionLastTime > WIFI_CONNECTION_INTERVAL)
      {
        Serial.println("Start WiFiManager => 192.168.4.1");

        wifiManager.resetSettings();
        bool wmRes = wifiManager.autoConnect("SERVO_SWITCH_1");
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.printf("\nSuccess\n");
          mySPIFFS->saveConfig(LittleFS, wifiManager.getConfiguredSTASSID(), wifiManager.getConfiguredSTAPassword());
          delay(100);
          ESP.restart();

          break;  // Temp
        }
      }

  }
  
  
  
  Serial.printf("\n%s\r\n", "Start MQTT Setup");

  client.setServer(mqtt_server, 1883);  // Sets the server details. 
  client.setCallback(callback);  // Sets the message callback function.  

  // lightServo.detach();
  // M5.IMU.begin();
  // Serial.printf("0x%02x\n", M5.IMU.whoAmI());




}



void loop() {

  if (!client.connected()) {
        reConnect();
  }
  client.loop();



  localSwitch();
  rotateServo(targetPos, 5);
 



  // 1초마다 상태 갱신
  if (millis() - lastTime > interval)
  {
    lastTime = millis();

    // Serial.printf("Flow : %d\r\n", flow++);
    if(!whileCallback)
    {
      client.publish(light_topic, msg);
    }

    if(lastTime > restartTime)
    {
      ESP.restart();
    }
  }


}







void forever(void) {
    while (true) {
        delay(1);
    }    
}


void callback(char* topic, byte* payload, unsigned int length) {

    whileCallback = true;

    Serial.printf("[%5d] Message arrived [", flow++);
    Serial.print(topic);
    Serial.print("] : ");
    Serial.print("(");
    Serial.print(length);
    Serial.print(") -> ");
    memcpy( &receivedMsg, payload, length);
    memcpy( &msg, &receivedMsg, MSG_BUFFER_SIZE);
    Serial.printf("Data : %d =>> %s", lightState, receivedMsg);
    Serial.println();
    
    
    
    if(!strcmp(receivedMsg, lastMsg))
    {
      posChanged = false;
      Serial.println("Value Same!");
    }
    else
    {
      posChanged = true;
      Serial.println("Value Changed!");
      if(!lightServo.attached())
      {
        lightServo.attach(SERVO_PIN);
        Serial.println("----Attach----");
      }
    }


    if(!strcmp(receivedMsg, "ON"))
    {
      if (!lightState)
      {
        Serial.println("turn on Light");
        targetPos = SERVO_ON_ANGLE;
        
      }
      
    }
    else if (!strcmp(receivedMsg, "OFF"))
    {
      if (lightState)
      {
        Serial.println("turn off Light");
        targetPos = SERVO_OFF_ANGLE;
      }
    }

    lightState = !lightState;
    
    Serial.printf("TARGET POS => %d\r\n", targetPos);
    memcpy( &lastMsg, &receivedMsg, MSG_BUFFER_SIZE);

    memset(&receivedMsg, 0x00, length);

    whileCallback = false;

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
            client.publish(light_topic, "hello world"); // publish To Connect

            client.subscribe(light_topic);    // Subsrcibe
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}


void localSwitch()
{
  myBtn.read();
  
  if (myBtn.wasPressed() || myBtn.pressedFor(1000, 200)) {
      Serial.printf("BUTTON Pressed  200\r\n");

      if(lightState)
      {
        snprintf(msg, MSG_BUFFER_SIZE, "ON");
      }
      else
      {
        snprintf(msg, MSG_BUFFER_SIZE, "OFF");        
      }
      lightState = !lightState;

  }
  else if(myBtn.wasReleasefor(700))
  {
    Serial.printf("BUTTON Released  700\r\n");
  }
  else
  {
    // Serial.printf("BUTTON %d | %d \r\n", myBtn.isPressed());
  }
  
}

void rotateServo(int _targetPos, uint8_t _delay)
{
  if(millis() - servoTime > _delay)
  {
    servoTime = millis();
    
    if(currentServoPos == _targetPos)
    {

        if(lightServo.attached())
        {
          Serial.println("-----Detach----");
          if(posChanged)
          {
            lightServo.detach();
          }
          delay(100);
          // lightServo.detach();
        }        
        
    }
    else if( currentServoPos > _targetPos) // 30 ->
    {
        lightServo.write(currentServoPos--);
        Serial.printf("CURRENT : %d\r\n", currentServoPos);
    }
    else if (currentServoPos < _targetPos)
    {
        lightServo.write(currentServoPos++);
        Serial.printf("CURRENT : %d\r\n", currentServoPos);
    }

  }
  
}