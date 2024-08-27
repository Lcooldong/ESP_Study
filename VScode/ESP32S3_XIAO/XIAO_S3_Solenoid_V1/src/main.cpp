#include <Arduino.h>
#include <SoftwareSerial.h>

#include "MyLittleFS.h"
#include "MyOTA.h"

#include "Button.h"
#include "config.h"



const int ledChannel1 = 0;
const int ledChannel2 = 1;
const int ledFreq = 5000;
const int ledResolution = 8;


const int RS485_RX = D0;
const int RS485_TX = D1;

const int RELAY_1_PIN = D2;
const int RELAY_2_PIN = D3;

const int SDA_PIN = D4;
const int SCL_PIN = D5;

const int LIGHT_PIN = D8;

const int PHOTO_SENSOR_PIN = D9;
const int BUTTON_PIN = D10;

// const int RS485_RX = GPIO_NUM_44;
// const int RS485_TX = GPIO_NUM_43;



uint64_t breathingTime = 0;
bool breathingDirection = false;
int breathingValue = 0;

uint64_t lastTime = 0;
uint64_t count = 0;


int lightState = 0;
int otaStatus = 0;
int otaStatusCount = 0;

uint64_t rs485Time = 0;
int ledValue = 0;

MyLittleFS* myFS = new MyLittleFS();
Button myBtn(BUTTON_PIN, 0, 10);  // 0 -> HIGH 

int photoState = 0;
int photoFlag = 0;
uint64_t photoTime = 0;
int photoCount = 0;

void printKoreanTime();
void pressKey();
void breathe(uint8_t _delay);
void localSwitch();
void setRelay1();
void setRelay2();
void photoSensing();

MyOTA* myOTA = new MyOTA();
// HardwareSerial RS485(Serial0);
EspSoftwareSerial::UART RS485;

void setup() {
  Serial.begin(115200);
  // RS485.begin(115200, SERIAL_8N1, RS485_RX, RS485_TX);
  RS485.begin(38400, EspSoftwareSerial::SWSERIAL_8N1, RS485_RX, RS485_TX);

  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);


  ledcSetup(ledChannel1, ledFreq, ledResolution);
  ledcAttachPin(LED_BUILTIN, ledChannel1); 

  ledcSetup(ledChannel2, ledFreq, ledResolution);
  ledcAttachPin(LIGHT_PIN, ledChannel2); 
  
  for (int i = 0; i < 3; i++)
  {
    ledcWrite(ledChannel1, 255);
    delay(500);
    ledcWrite(ledChannel1, 0);
    delay(500);
  }
  
  delay(100);

  // Find button Status
  // while (true)
  // { 
  //   myBtn.read();
  //   if(myBtn.pressedFor(3000,5000))
  //   {
  //     otaStatus = 1;
  //     Serial.printf("STATUS : %d ------------------------\r\n", otaStatus);
  //     break;
  //   }
  //   else if (myBtn.wasReleased()) 
  //   {
  //     otaStatus = 0;
  //     Serial.printf("STATUS : %d ------------------------\r\n", otaStatus);
  //     break;
  //   }
  // }

  


  
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

  if(millis() - rs485Time > 1)
  {
    if(RS485.available())
    {
      char cmd = RS485.read();

      switch (cmd)
      {
      case '1':
        Serial.printf("RS485 : %c\r\n", cmd);
        setRelay1();
        break;
      
      case '2':
        Serial.printf("RS485 : %c\r\n", cmd);
        setRelay2();
        break;
      case '3':        
        if(ledValue >= 255)
        {
          ledValue = 255;
        }
        else
        {
          ledValue++;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, ledValue);
        ledcWrite(ledChannel2 , ledValue);
        break;
      case '4':
        if(ledValue <= 0)
        {
          ledValue = 0;
        }
        else
        {
          ledValue--;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, ledValue);
        ledcWrite(ledChannel2 , ledValue);
        break;

      default:
        break;
      }

    }
    rs485Time = millis();
  }
 

  if(millis() - lastTime > 1000)
  {
    lastTime = millis();
    count++;
    // Serial.printf("[%d]\r\n", count);
   
    
    String buffer = "";

    buffer += "[";
    buffer += (String)count;
    buffer += "] | S :";
    buffer += (String)lightState;
    buffer += " | ";
    buffer += "] | OTA ";
    buffer += (String)otaStatus;
    buffer += " |\r\n";

    // printKoreanTime();
    Serial.print(buffer);
    
    // Serial.flush();
    // RS485.print(buffer);
    byte* textArray = new byte[buffer.length()]; // + 1 NULL
    for (int i = 0; i < buffer.length(); i++)
    {
      textArray[i] = buffer[i];
    }
    

    // buffer.getBytes(textArray, buffer.length() + 1);

    for (int i = 0; i < buffer.length(); i++)
    {
      RS485.write(textArray[i]);
    }
    
    // RS485.write(textArray, buffer.length() + 1);

    // Serial.printf(" [%d] = %d\r\n", buffer.length() + 1, sizeof(textArray));
    // RS485.println(count);
    
    RS485.flush();
    
    // WebSerial.printf("%d : \r\n", count);
    
  }

  // localSwitch();
  photoSensing();
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
        setRelay1();
        break;         
      }
  
      case '2':
      {
        Serial.println("2->");
        setRelay2();
        break;
      }

      case '3':
        Serial.println("3->");
        photoFlag = !photoFlag;
        Serial.printf("Photo Flag : %d\r\n", photoFlag); 
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
    

    ledcWrite(ledChannel1, breathingValue);

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


void setRelay1()
{
  digitalWrite(RELAY_1_PIN, LOW);
  delay(100);
  digitalWrite(RELAY_1_PIN, HIGH);
  delay(100);
  digitalWrite(RELAY_1_PIN, LOW);
  delay(100);
  // myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0),10, 1 );
  Serial.printf("Push Solenoid\r\n");
}

void setRelay2()
{
  digitalWrite(RELAY_2_PIN, LOW);
  delay(100);
  digitalWrite(RELAY_2_PIN, HIGH);
  delay(100);
  digitalWrite(RELAY_2_PIN, LOW);
  delay(100);
  // myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255),10, 1 );
  Serial.printf("Release Solenoid\r\n");
}

void photoSensing()
{
  if(photoFlag)
  {
    if(millis() - photoTime > 10)
    {
      photoTime = millis();
      int photoValue = digitalRead(PHOTO_SENSOR_PIN);
      Serial.printf("PHOTO : %d\r\n", photoValue);
      RS485.printf("PHOTO : %d\r\n", photoValue);
      if(photoValue)
      {
        photoCount++;
      }
      else
      {
        photoCount = 0;
      }
      

      if(photoCount > 10)
      {
        photoState = 1;
        photoFlag = 0;
        photoCount = 0;
        Serial.printf("해당 위치에 도달하였습니다!");
      }
      else
      {
        photoState = 0;
        
      }
    
    }
  }
}