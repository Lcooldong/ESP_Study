#include <Arduino.h>
#include <PubSubClient.h>


#include <ESPAsyncWiFiManager.h>
#include <ElegantOTA.h>
#include <WebSerial.h>

#include <ESP32Servo.h>
#include <time.h>

#include <U8x8lib.h>
#include <Wire.h>

#include "Button.h"
#include "MyLittleFS.h"
#include "config.h"

MyLittleFS* mySPIFFS = new MyLittleFS();

AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;
DNSServer dns;
AsyncWiFiManager wifiManager(&server,&dns);
WiFiClient espClient;
PubSubClient client(espClient);


const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = KOR_GMT_9;
const int daylightOffset_sec = 0;

String formattedDate;
String yearStamp;
String dayStamp;
String timeStamp;


Servo lightServo;
int targetPos = 0;
bool posChanged = false;
int currentServoPos = 0;
unsigned long servoTime = 0;
int servoIdleCount = 0;

Button myBtn(SWITCH_PIN, 0, 10);  // 0 -> HIGH 


const char* mqtt_server = "mqtt.m5stack.com";
const char* light_topic = "M5Stack/LCD/ESP32C3/MYROOM/SERVO_STATE";


#define MSG_BUFFER_SIZE (8)
char msg[MSG_BUFFER_SIZE] = "NOT";
char lastMsg[MSG_BUFFER_SIZE];
char receivedMsg[MSG_BUFFER_SIZE];


unsigned long flow = 0;
bool lightState = false;
bool whileCallback = false;

unsigned long releaseTime = 0;
unsigned long buttonTime = 0;
bool btnPressing = false;
bool btnRelease = false;


int count = 0;
unsigned long lastTime = 0;
unsigned int interval = 1000;
unsigned long restartTime = 4000000000;
unsigned long beforeShutdown = 0;
unsigned int shutdownTimeOut = 10000;

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL_PIN, /* data=*/ SDA_PIN, /* reset=*/ U8X8_PIN_NONE);

void init_WiFi();
void beginWiFiManager();
void forever();
void callback(char* topic, byte* payload, unsigned int length);
void recvMsg(uint8_t *data, size_t len);
void reConnect();
void localSwitch();
void rotateServo(int _targetPos, uint8_t _delay);
void init_u8x8(const uint8_t* _font);
void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);
void printLocalTime();
void printKoreanTime();
void servoAttach();

void setup() {

 // myLittleFS->InitSPIFFS();
  Serial.begin(115200);   

  pinMode(SWITCH_PIN, INPUT);
  pinMode(SUB_GND, OUTPUT);
  digitalWrite(SUB_GND, LOW);
  // lightServo.attach(SERVO_PIN);
  
  mySPIFFS->InitLitteFS();
  init_u8x8(u8x8_font_chroma48medium8_r);

  delay(100);

  mySPIFFS->listDir(LittleFS, "/", 0);
  Serial.println("Connecting to WiFi...");

  if(mySPIFFS->loadConfig(LittleFS))
  {
    init_WiFi();
  }
  else
  {
    //const char* ssid        = "Cooldong";
    //const char* password    = "8ec4hkx000";
    //WiFi.begin(ssid, password);
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }
  delay(500);
  beginWiFiManager();
  mySPIFFS->loadMotorValue(LittleFS);
  targetPos = SERVO_IDLE_ANGLE;
  mySPIFFS->loadLightState(LittleFS);
  lightState = mySPIFFS->mylightState;
  Serial.printf("CURRENT STATE : %d\r\n", lightState);

  
  // if(lightState)
  // {
  //   strcpy(msg, "ON");
  // }
  // else
  // {
  //   strcpy(msg, "OFF");
  // }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // printLocalTime();


  
  Serial.printf("\n%s\r\n", "Start MQTT Setup");

  client.setServer(mqtt_server, 1883);  // Sets the server details. 
  client.setCallback(callback);  // Sets the message callback function.  

  WebSerial.onMessage([](const String& msg) { Serial.println(msg); }); 
  WebSerial.begin(&server);
  WebSerial.setBuffer(128);
  server.onNotFound([](AsyncWebServerRequest* request) { request->redirect("/webserial"); });


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is ElegantOTA AsyncDemo.");
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  
  server.begin();
  Serial.printf("\r\n===============Start ESP32===============\r\n");

  lightServo.attach(SERVO_PIN);
}



void loop() {

  if (!client.connected()) {
        reConnect();
  }
  client.loop();
  ElegantOTA.loop();

  //localSwitch();
  rotateServo(targetPos, 1);
 
  
  // 1초마다 상태 갱신
  if ( millis() - lastTime > interval  && !btnPressing)
  {
    lastTime = millis();

    printKoreanTime();

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








void init_WiFi()
{
    Serial.println(mySPIFFS->ssid);
    Serial.println(mySPIFFS->pass);

    WiFi.mode(WIFI_STA); // 
    WiFi.begin(mySPIFFS->ssid, mySPIFFS->pass);
    //delay(3000);
    Serial.printf("IP : %s\r\n", WiFi.localIP().toString().c_str());
    Serial.println("Connect to Flash Memory");
    u8x8.setCursor(0, 0);
    u8x8.print("IP:");
    u8x8.setCursor(3, 0);
    u8x8.print(WiFi.localIP());
}

void beginWiFiManager()
{
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
}




void forever(void) {
    while (true) {
        delay(1);
    }
}


void callback(char* topic, byte* payload, unsigned int length) {

    whileCallback = true;
    flow++;
    Serial.printf("[%05d] Message arrived [", flow);
    Serial.print(topic);
    Serial.print("] : ");
    Serial.print("(");
    Serial.print(length);
    Serial.print(") -> ");
    memcpy( &receivedMsg, payload, length);
    memcpy( &msg, &receivedMsg, MSG_BUFFER_SIZE);
    
    Serial.printf("Data : %d =>> %s\r\n", lightState, receivedMsg);
    
    WebSerial.print("[");
    WebSerial.print(flow);
    WebSerial.print("] -> [");
    // WebSerial.printf("[%05d] Message arrived [", flow);
    WebSerial.print(topic);
    WebSerial.print("] : ");
    WebSerial.print("(");
    WebSerial.print(length);
    WebSerial.print(") -> ");
    WebSerial.print("Data : ");
    WebSerial.print(lightState);
    WebSerial.print("=>>");
    WebSerial.println(msg);
    // WebSerial.printf("Data : %d =>> %s\r\n", lightState, receivedMsg);
    
    
    if(!strcmp(receivedMsg, lastMsg))
    {
      posChanged = false;
#ifdef DEBUG
      Serial.println("Value Same!");
#endif
    }
    else
    {
      posChanged = true;
#ifdef DEBUG
      Serial.println("Value Changed!");
#endif
      //servoAttach();
    }

    if(posChanged)
    {
      servoAttach();
      if(!strcmp(msg, "ON"))
      {
#ifdef DEBUG
          Serial.println("turn on Light");
#endif
          targetPos = SERVO_ON_ANGLE;
          lightState = true;
          
      }
      else if (!strcmp(msg, "OFF"))
      {

#ifdef DEBUG
          Serial.println("turn off Light");
#endif
          targetPos = SERVO_OFF_ANGLE;
          lightState = false;
      }

      servoIdleCount = 0;

      mySPIFFS->saveCurrentState(LittleFS, lightState);
    }
    else
    {
      servoIdleCount++;

      if(servoIdleCount > SERVO_IDLE_DELAY)
      {
        //servoAttach();
        targetPos = SERVO_IDLE_ANGLE;
      }
    }
    mySPIFFS->saveMotorValue(LittleFS, targetPos);
    Serial.println("Save Motor value");



#ifdef DEBUG    
    Serial.printf("TARGET POS => %d\r\n", targetPos);
#endif
    memcpy( &lastMsg, &receivedMsg, MSG_BUFFER_SIZE);

    memset(&receivedMsg, 0x00, length);

    whileCallback = false;

}

void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
  if (d == "ON"){
    // digitalWrite(LED, HIGH);
  }
  if (d=="OFF"){
    // digitalWrite(LED, LOW);
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

  if(myBtn.pressedFor(3000,10000))
  {
    Serial.printf("BUTTON PressedFor 3000\r\n");

  } 
  else if(myBtn.wasReleasefor(500))
  {
    Serial.printf("BUTTON Released  1000\r\n"); // Button Pressing Filter

  } 
  else if (myBtn.wasReleased()) 
  {
      Serial.printf("BUTTON Pushed \r\n");

      if(!lightState)
      {
        Serial.printf("[ Light ON : BUTTON ]\r\n");
        snprintf(msg, MSG_BUFFER_SIZE, "ON");
        // memcpy(msg, "ON", MSG_BUFFER_SIZE);
        
        lightState = true;
      }
      else
      {
        Serial.printf("[ Light OFF : BUTTON ]\r\n");
        snprintf(msg, MSG_BUFFER_SIZE, "OFF");   // 여러 포멧 가능
        // memcpy(msg, "OFF", MSG_BUFFER_SIZE);  // 문자열 빠름
        lightState = false; 
      }
      
      Serial.printf("%s\r\n", msg);
      client.publish(light_topic, msg); // Sync 가 안맞음
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
#ifdef DEBUG
          Serial.println("-----Detach----");
#endif
          if(servoIdleCount > SERVO_IDLE_DELAY)
          {
            lightServo.detach();
          }

          // delay(100);
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


void init_u8x8(const uint8_t* _font)
{
  u8x8.begin();
  u8x8.setPowerSave(0);  
  u8x8.setFont(_font);
  u8x8.setInverseFont(0);
}

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}

void printKoreanTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print(&timeinfo,  "%Y/%B/%d(%A) %H:%M:%S ||");
  
  // int month = timeinfo.tm_mon;
  // Serial.printf("MONTH %d\r\n",  month);
}

void servoAttach()
{
    if(!lightServo.attached())
    {
    lightServo.attach(SERVO_PIN);
#ifdef DEBUG
      Serial.println("----Attach----");
#endif
    }
}