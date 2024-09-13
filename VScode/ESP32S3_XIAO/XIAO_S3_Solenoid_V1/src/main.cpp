#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "MyLittleFS.h"
#include "MyOTA.h"
#include "MyModbus.h"

#include "Button.h"
#include "config.h"



#define SLAVE_ID 1
#define VERSION  2 
#define OLED_128X32_ADDRESS 0x3C
#define RS485_BAUDRATE 38400


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

bool oledState = false;
uint64_t lastTime = 0;
uint64_t count = 0;

uint64_t lightTime = 0;
int lightState = 0;
uint8_t lightValue = 0;

int otaStatus = 0;
int otaStatusCount = 0;

uint16_t lastRelayValue = 0;
bool relayState = false;
uint64_t rs485Time = 0;


int photoState = 0;
int photoFlag = 0;
uint64_t photoTime = 0;
int photoCount = 0;

EspSoftwareSerial::UART RS485;

MyOTA* myOTA = new MyOTA();
MyLittleFS* myFS = new MyLittleFS();
MyModbus* myModbus = new MyModbus(&RS485, SLAVE_ID);
Button myBtn(BUTTON_PIN, true, 20);  // 0 -> HIGH , Pull-up ->Invert (true) 
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE);





void initPinout();
void initOLED();
void printKoreanTime();
void pressKey();
void breathe(uint8_t _delay);
void localSwitch();
void setRelay1();
void setRelay2();

void controlSolenoid();
void setLED();
void photoSensing();

void rs485Command();
void connectOLED(uint8_t address);
void setOLED();
// HardwareSerial RS485(Serial0);

void setup() {
  Serial.begin(115200);
  // RS485.begin(115200, SERIAL_8N1, RS485_RX, RS485_TX);
  RS485.begin(RS485_BAUDRATE, EspSoftwareSerial::SWSERIAL_8N1, RS485_RX, RS485_TX);

  initPinout();
  initOLED();


  // Find button Status
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
    else if(myBtn.isReleased()) // Current 
    {
      Serial.printf("STATUS : released %d ------------------------\r\n", otaStatus);
      break;
    }
  }

  if(otaStatus)
  {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 16);
    u8g2.printf("[%d] Solenoid V%d", SLAVE_ID, VERSION);
    u8g2.setCursor(0, 32);
    u8g2.printf("OTA Mode Start");
    u8g2.sendBuffer();

    myOTA->initOTA();

    // struct tm timeinfo;
    // if(!getLocalTime(&timeinfo)){
    //   WebSerial.println("Failed to obtain time");
    //   return;
    // }

    // WebSerial.print(&timeinfo, "%Y/%B/%d(%A) %H:%M:%S :: ");
    WebSerial.println("OTA Start");
    Serial.println("OTA Start");

    delay(1000);

    u8g2.clearBuffer();
    u8g2.setCursor(0, 16);
    u8g2.printf("[%d] Solenoid V%d", SLAVE_ID, VERSION);
    u8g2.setCursor(0, 32);
    u8g2.printf("IP : %s", WiFi.localIP().toString().c_str());
    u8g2.sendBuffer();

  }
  else
  {
    myFS->InitLitteFS();

    delay(1000);

    myFS->listDir(LittleFS, "/", 0);
    // myFS->saveSol(LittleFS, 255, 100);
    delay(100);
    myFS->loadSol(LittleFS);
  }
}

void loop() {

  
 

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
    // Serial.print(buffer);
    
    // Serial.flush();
    // RS485.print(buffer);
    byte* textArray = new byte[buffer.length()]; // + 1 NULL
    for (int i = 0; i < buffer.length(); i++)
    {
      textArray[i] = buffer[i];
    }
    connectOLED(OLED_128X32_ADDRESS);
    if(otaStatus)
    {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 16);
      u8g2.printf("[%d] Solenoid V%d", SLAVE_ID, VERSION);
      u8g2.setCursor(0, 32);
      u8g2.printf("IP : %s", WiFi.localIP().toString().c_str());
      u8g2.sendBuffer();
    }   

  }

  localSwitch();
  breathe(5);
  if(otaStatus)
  {
    myOTA->loop();
  }
  else
  {
    myModbus->pollModbus();
    controlSolenoid();
    setLED();
    photoSensing();
    pressKey(); // Debug
  }
}






void initPinout()
{
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(PHOTO_SENSOR_PIN, INPUT);

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
}

void initOLED()
{
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 16);
  u8g2.printf("[%d] Solenoid V%d", SLAVE_ID, VERSION);
  u8g2.setCursor(0, 32);
  u8g2.printf("--Start--");
  u8g2.sendBuffer();
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

      case '5':
        if(lightValue >= 255)
        {
          lightValue = 255;
        }
        else
        {
          lightValue++;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, lightValue);
        ledcWrite(ledChannel2 , lightValue);
        break;
      case '6':
        if(lightValue <= 0)
        {
          lightValue = 0;
        }
        else
        {
          lightValue--;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, lightValue);
        ledcWrite(ledChannel2 , lightValue);
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

      if(!relayState)
      {
        
        // snprintf(msg, MSG_BUFFER_SIZE, "ON");
        // memcpy(msg, "ON", MSG_BUFFER_SIZE);
        
        // lightState = 1;
        // setRelay1();
        myModbus->holdingRegisters[1] = 0x01;
        Serial.println("Button -> Relay 1");
        // Serial.printf("[ Light ON : BUTTON ] -> %d \r\n", lightState);
      }
      else
      {
        
        // snprintf(msg, MSG_BUFFER_SIZE, "OFF");   // 여러 포멧 가능
        // memcpy(msg, "OFF", MSG_BUFFER_SIZE);  // 문자열 빠름
        // lightState = 0; 
        // setRelay2();
        myModbus->holdingRegisters[1] = 0x02;
        Serial.println("Button -> Relay 2");
        // Serial.printf("[ Light OFF : BUTTON ] -> %d\r\n", lightState);
      }
      setOLED();
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
  // delay(100);
  myFS->saveSol(LittleFS, myModbus->holdingRegisters[1], myModbus->holdingRegisters[3]);
  relayState = true;
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
  // delay(100);
  myFS->saveSol(LittleFS, myModbus->holdingRegisters[1], myModbus->holdingRegisters[3]);
  relayState = false;
  // myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255),10, 1 );
  Serial.printf("Release Solenoid\r\n");
}

void controlSolenoid()
{
  if(myModbus->holdingRegisters[1] != lastRelayValue)
  {
    lastRelayValue = myModbus->holdingRegisters[1];
    
    if(myModbus->holdingRegisters[1] == 0x01)
    {
      setRelay1();
      
    }
    else if(myModbus->holdingRegisters[1] == 0x02)
    {
      setRelay2();
      
    }


    setOLED();

    // myModbus->holdingRegisters[1] = 0x00; // Execute Once
  }
}

void setLED()
{
  if(myModbus->holdingRegisters[2])
  {
    if(millis() - lightTime > 100)
    {
      ledcWrite(ledChannel2 , myModbus->holdingRegisters[3]);
      myFS->saveSol(LittleFS, myModbus->holdingRegisters[1], myModbus->holdingRegisters[3]);

      setOLED();

      myModbus->holdingRegisters[2] = 0;  // END
      lightTime = millis();
    }
    
  }
}

void photoSensing()
{
  // if(photoFlag)
  if(myModbus->holdingRegisters[4])
  {
    if(millis() - photoTime > 50)
    {
      photoTime = millis();
      int photoValue = digitalRead(PHOTO_SENSOR_PIN);
      // Serial.printf("PHOTO : %d\r\n", photoValue);
      // RS485.printf("PHOTO : %d\r\n", photoValue);
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
        photoState = myModbus->holdingRegisters[4] + 0x01;  // 2
        // photoFlag = 0;
        photoCount = 0;
        Serial.printf("해당 위치에 도달하였습니다!\r\n");
        myModbus->holdingRegisters[4] = 0;
      }
      else
      {
        photoState = 0x03;
        Serial.printf("초기화 중\r\n");
      }
      myModbus->holdingRegisters[5] = photoState;
    }
  }
}


void rs485Command()
{
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
        if(lightValue >= 255)
        {
          lightValue = 255;
        }
        else
        {
          lightValue++;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, lightValue);
        ledcWrite(ledChannel2 , lightValue);
        break;
      case '4':
        if(lightValue <= 0)
        {
          lightValue = 0;
        }
        else
        {
          lightValue--;
        }
        Serial.printf("RS485 : [%c] => %d\r\n", cmd, lightValue);
        ledcWrite(ledChannel2 , lightValue);
        break;

      default:
        break;
      }

    }
    rs485Time = millis();
  }

}

void connectOLED(uint8_t address){
  Wire.begin();
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  if (error == 0) 
  {
    // Serial.println("OLED Connected");
    if(!oledState)
    {
      u8g2.begin();
      oledState = true;
    }
  }
  else
  {
    // Serial.println("OLED Disconnected");
    oledState = false;    
  }
  Wire.end();
}

void setOLED()
{
  u8g2.clearBuffer();
  // u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 16);
  u8g2.printf("[%d] Solenoid V%d", SLAVE_ID, VERSION);
  u8g2.setCursor(0, 32);
  u8g2.printf("SOL [0x%02d]|LED [%3d]|", myModbus->holdingRegisters[1], myModbus->holdingRegisters[3]);
  u8g2.sendBuffer();

}