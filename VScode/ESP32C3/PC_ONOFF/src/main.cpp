#ifdef ESP32
  #include <WiFi.h>
  #include <Wire.h>
  #include <U8g2lib.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
//#define WEBSERVER_H
#include <ESPAsyncWebServer.h>
//#include  <WiFiManager.h>
#include <ESPAsyncWiFiManager.h>
#include <Servo.h>


#define UNSIGNED_LONG_LIMIT 4000000000
#define RECONNECT_INTERVAL 10000
#define ON_OFF_INTERVAL 2000
#define PC_ON_ANALOG_VALUE 0
// #define PC_ON_ANALOG_VALUE 4000

const char* PARAM_INPUT_1 = "state";

const int output = 4;
const int servoPin = 21;
const int buttonPin = 3;
const int relay = 20;
const int volt_input = 1;

#include "neopixel.h"
#include "MyLittleFS.h"

// Variables will change:
int ledState = LOW;          // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
bool onFlag = false; 
bool offFlag = false;
bool shutdownFlag = false;
bool wmRes = false;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

unsigned long reconnectTime = 0;
unsigned long lastOnOffTime = 0;
unsigned long wifiCurrentTime = 0;

// Create AsyncWebServer object on port 80
//WiFiManager wm;
DNSServer dns;
AsyncWebServer WiFi_server(80); //  WiFi Manager

Servo myservo;
AsyncWebServer server(5000);  // 스위치 서버
AsyncEventSource events("/events");
MyNeopixel* myNeopixel = new MyNeopixel();
MyLittleFS* myLittleFS = new MyLittleFS();
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

String outputState();
String processor(const String& var);
void turnOnOffRelay();
void shutdownRelay();
void showLcdText(int cursor_x, int cursor_y, String _text);
void InitU8g2();
void configModeCallback (AsyncWiFiManager *myWiFiManager);


void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(output, OUTPUT); 
  pinMode(relay, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(volt_input, INPUT_PULLUP);
  myservo.attach(servoPin);
  myLittleFS->InitLitteFS();
  myNeopixel->InitNeopixel();
  InitU8g2();
  digitalWrite(relay, LOW);
  //digitalWrite(output, LOW);
  // for (int i = 0; i < 10; i++)
  // {
  //   if(analogRead(volt_input) > PC_ON_ANALOG_VALUE)
  //   {
  //     digitalWrite(output, HIGH);
  //     myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0), 50, 50);
  //   }
  //   else
  //   {
  //     digitalWrite(output, LOW);
  //     myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 50);
  //   }
  // }

  // KT
  // IPAddress ip (172, 30, 1, 41);  // M5stamp -> 40, LCD 0.42-> 41
  // IPAddress gateway (172, 30, 1, 254);
  // IPAddress subnet(255, 255, 255, 0);


  // ASUS
  IPAddress ip (192, 168, 50, 40);  // M5stamp -> 40, LCD 0.42-> 41
  IPAddress gateway (192, 168, 50, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);

  if(myLittleFS->loadConfig())
  {
    Serial.println(myLittleFS->ssid);  
    Serial.println(myLittleFS->pass);

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(myLittleFS->ssid, myLittleFS->pass);
  }
  else
  {
    //const char* ssid = "Cooldong";
    //const char* password = "8ec4hkx000";
    //WiFi.begin(ssid, password);
    Serial.println("Saved file doesn't exist => Move to WiFiManager");
  }
  


  // Connect to Wi-Fi
  AsyncWiFiManager wm(&WiFi_server,&dns);
  wm.setAPCallback(configModeCallback);  

  long lastTime = millis();
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
    wifiCurrentTime = millis();
    if (wifiCurrentTime - lastTime > 10000)
    {
        
        //WiFi.disconnect();
        myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255), 50, 50);
        u8g2.clearBuffer();
        showLcdText(0, 10 , "192.168.4.1");
        Serial.printf("\nEnter : [] %s ]with your browser\n", WiFi.softAPIP().toString());
        wm.resetSettings();

        wmRes = wm.autoConnect("ESP32C3_WiFiManager");
        if(!wmRes)
        {
          Serial.println("Failed to connect");
        }
        else
        {
          Serial.println("connected...yeey :)");
          // 새로운 SSID, PASS 쓰기

          // myLittleFS->saveConfig(wm. getWiFiSSID(), wm.getWiFiPass());
          myLittleFS->saveConfig(wm.getConfiguredSTASSID(), wm.getConfiguredSTAPassword());
          delay(100);
          // myLittleFS->writeFile(LittleFS, "/config.txt", "Hello C3");
          WiFi_server.end();
          ESP.restart();
          
          break;
        }  
    }

  }

  Serial.println();
  Serial.print(F("WiFi Connected -> IP : "));
  Serial.println(WiFi.localIP());
  u8g2.clearBuffer();
  showLcdText(0, 10 , myLittleFS->ssid);
  showLcdText(0, 20 , "Connected");
  WiFi_server.end();
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html", false, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      digitalWrite(output, inputMessage.toInt()); // 밑 digitalRead 와 연관있음

      if(inputMessage.toInt())
      {
        onFlag = true;
      }
      else
      {
        offFlag = true;
      }

      
      //myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0), 50 * inputMessage.toInt(), 50);
      ledState = !ledState;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.printf("INPUT MESSAGE : %s\n", inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output)).c_str());  // 보드 -> Javascript
  });

  server.on("/shutdown", HTTP_GET,[] (AsyncWebServerRequest *request){
    request->send_P(200, "text/html", "SHUTDOWN");
    shutdownFlag = true;
    
  });

  // Start server
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
    }
    //send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!",NULL,millis(),10000);
  });
  //HTTP Basic authentication
  server.addHandler(&events);
  server.serveStatic("/", LittleFS, "/");
  server.begin();
  Serial.printf("Server Started\n");
}


void loop() {

  int reading = digitalRead(buttonPin);
  
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  lastButtonState = reading;

  // WiFi Reconnect
  shutdownRelay();
  turnOnOffRelay();
  

  if (WiFi.status ()== WL_CONNECTED)
  {
    if ((millis() - lastOnOffTime) > ON_OFF_INTERVAL)
    {
      events.send("ping",NULL,millis());

      // if(analogRead(volt_input) > PC_ON_ANALOG_VALUE)
      // {
      //   digitalWrite(output, HIGH);
      //   // On 이벤트 잘 작동 X -> 처음 킬 때는 잘 작동함
      //   events.send(String("On").c_str(), "current_on", millis());
      //   myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 255), 5, 50);
      //   showLcdText(0, 40 , "Status:[ On ]");      
      // }
      // else
      // {
      //   digitalWrite(output, LOW);
      //   Serial.println("EVENT");
      //   events.send(String("Off").c_str(), "current_off", millis());
      //   myNeopixel->pickOneLED(0, myNeopixel->strip->Color(200, 50, 0), 5, 50);
      //   showLcdText(0, 40 , "Status:[ OFF ]");
      //   delay(500);
      // }
      lastOnOffTime = millis();

      if (lastOnOffTime  > UNSIGNED_LONG_LIMIT)
      {
        ESP.restart();
      } 
    }
  }
  else
  {
    if(millis() - reconnectTime > RECONNECT_INTERVAL)
    {
      Serial.println("WiFi Disconnected");
      WiFi.reconnect();
      if(WiFi.status() != WL_CONNECTED)
      {
        ESP.restart();
      }
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 50);
      reconnectTime = millis();
      if (reconnectTime > UNSIGNED_LONG_LIMIT)
      {
        ESP.restart();
      }
    }
  }
}


String outputState(){
  if(digitalRead(output))
  {
    return "checked";
  }
  else 
  {
    return "";
  }
  return "";
}


// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    buttons+= "<h4>Output - State <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
    return buttons;
    //return outputStateValue;
  }
  // else if(var == "CURRENT")
  // {
  //   String outputStateValue;
  //   if ( analogRead(volt_input) > 4000)
  //   {
  //     outputStateValue = "On";
  //   }
  //   else
  //   {
  //     outputStateValue = "Off";
  //   }
    
  //   return outputStateValue;
  // }
  return String();
}

void turnOnOffRelay()
{

  if(onFlag)
  {
    myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0), 50, 50);
    digitalWrite(relay, HIGH);
    myservo.write(90);
    delay(50);

    onFlag = false;
  }
  else if(offFlag)
  {
    myNeopixel->pickOneLED(0, myNeopixel->strip->Color(50, 50, 50), 5, 50);
    digitalWrite(relay, HIGH);
    myservo.write(0);
    delay(50);
    offFlag = false;
  }
  else
  {
    digitalWrite(relay, LOW);
  }


}



void shutdownRelay()
{
  if(shutdownFlag){
    digitalWrite(relay, HIGH);
    delay(5000);
    // while(analogRead(volt_input) > PC_ON_ANALOG_VALUE)
    // {
    //   delay(100);
    // }
    digitalWrite(relay, LOW);
    myNeopixel->pickOneLED(0, myNeopixel->strip->Color(50, 50, 50), 5, 50);
    shutdownFlag = false;
  }
}

void InitU8g2()
{
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.clearBuffer(); 
  u8g2.setCursor(0,10);
  u8g2.print(F("Start"));
  u8g2.sendBuffer();
}

void showLcdText(int cursor_x, int cursor_y, String _text)
{
  u8g2.setCursor(cursor_x, cursor_y);
  u8g2.print(F(_text.c_str()));
  u8g2.sendBuffer();
}

void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255), 50, 50);

}
