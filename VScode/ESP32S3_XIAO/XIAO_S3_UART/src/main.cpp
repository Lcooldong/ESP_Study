#include <Arduino.h>

#include "MyLittleFS.h"
#include "MyOTA.h"

#include "Button.h"
#include "config.h"

const int ledChannel = 0;
const int ledFreq = 5000;
const int ledResolution = 8;
const int SWITCH_PIN = GPIO_NUM_44;


uint64_t breathingTime = 0;
bool breathingDirection = false;
int breathingValue = 0;

uint64_t lastTime = 0;
uint64_t count = 0;

int lightState = 0;
int otaStatus = 0;
int otaStatusCount = 0;

MyLittleFS* myFS = new MyLittleFS();
Button myBtn(SWITCH_PIN, 0, 10);  // 0 -> HIGH 

void printKoreanTime();
void pressKey();
void breathe(uint8_t _delay);
void localSwitch();


MyOTA* myOTA = new MyOTA();


void setup() {
  Serial.begin(115200);


  pinMode(SWITCH_PIN, INPUT);
  ledcSetup(ledChannel, ledFreq, ledResolution);
  ledcAttachPin(LED_BUILTIN, ledChannel); // GPIO21

  
  for (int i = 0; i < 3; i++)
  {
    ledcWrite(ledChannel, 255);
    delay(500);
    ledcWrite(ledChannel, 0);
    delay(500);
  }
  
  delay(100);


  while (true)
  {
    myBtn.read();
    if(myBtn.pressedFor(3000,5000))
    {
      otaStatus = 1;
      Serial.printf("STATUS : %d ------------------------\r\n", otaStatus);
      break;
    }
    else if (myBtn.wasReleased()) 
    {
      otaStatus = 0;
      Serial.printf("STATUS : %d ------------------------\r\n", otaStatus);
      break;
    }

    
    
  }

  


  
  if(otaStatus)
  {
    myOTA->initOTA();

    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      WebSerial.println("Failed to obtain time");
      return;
    }
    WebSerial.print(&timeinfo, "%Y/%B/%d(%A) %H:%M:%S :: ");

    WebSerial.println("OTA Start");
    
  }


} 

void loop() {

  if(millis() - lastTime > 1000)
  {
    count++;
    // Serial.printf("[%d]", count);

    
    String buffer = "";

    buffer += "\r\n[";
    buffer += (String)count;
    buffer += "] | STATE => ";
    buffer += (String)lightState;
    buffer += " | ";
    buffer += "] | OTA => ";
    buffer += (String)otaStatus;
    buffer += " | ";

    printKoreanTime();
    Serial.print(buffer);
    Serial.flush();
    
    // WebSerial.printf("%d : \r\n", count);
    lastTime = millis();
  }

  localSwitch();
  pressKey();
  breathe(5);
  if(otaStatus)
  {
    myOTA->loop();
  }
}


void printKoreanTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print(&timeinfo, "%Y/%B/%d(%A) %H:%M:%S :: ");
    
  // int month = timeinfo.tm_mon;
  // Serial.printf("MONTH %d\r\n",  month);
}


void pressKey()
{
  if(Serial.available())
  {
    char cmd = Serial.read();

    switch (cmd)
    {
      case '1':
      {
        Serial.println("1->");

        break;         
      }
  
      case '2':
      {
        Serial.println("2->");

        break;
      }

      case '3':
        Serial.println("3->");
        break;

      case '4':
        Serial.printf("LIGHT -> %d \r\n", lightState);
        break;

      case 'i':
        Serial.println(WiFi.localIP());
        break;
    }

  }
}




void breathe(uint8_t _delay)
{
  if(millis() - breathingTime > _delay)
  {
    breathingTime = millis();
    if(breathingDirection == true)
    {
      breathingValue++;
    }
    else
    {
      breathingValue--;
    }

    if(breathingValue >= 255)
    {
      breathingDirection = false;
    }
    else if(breathingValue <= 0) 
    {
      breathingDirection = true;
    }
    

    ledcWrite(ledChannel, breathingValue);

  }
}

void localSwitch()
{

  myBtn.read();

  if(myBtn.pressedFor(3000,5000))
  {
    Serial.printf("BUTTON PressedFor 3000\r\n");
    ESP.restart();
  } 
  else if(myBtn.wasReleasefor(500))
  {
    Serial.printf("BUTTON Released  500\r\n"); // Button Pressing Filter

  } 
  else if (myBtn.wasReleased()) 
  {
      Serial.printf("BUTTON Pushed \r\n");

      if(!lightState)
      {
        
        // snprintf(msg, MSG_BUFFER_SIZE, "ON");
        // memcpy(msg, "ON", MSG_BUFFER_SIZE);
        
        lightState = 1;
        Serial.printf("[ Light ON : BUTTON ] -> %d \r\n", lightState);
      }
      else
      {
        
        // snprintf(msg, MSG_BUFFER_SIZE, "OFF");   // 여러 포멧 가능
        // memcpy(msg, "OFF", MSG_BUFFER_SIZE);  // 문자열 빠름
        lightState = 0; 
        Serial.printf("[ Light OFF : BUTTON ] -> %d\r\n", lightState);
      }
      
      // Serial.printf("%s\r\n", msg);
      
  }
  
}

