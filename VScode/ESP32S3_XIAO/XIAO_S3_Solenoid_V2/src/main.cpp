#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "EspCAN.h"

#include "MyLittleFS.h"
#include "MyOTA.h"

#include "Button.h"
#include "config.h"



#define SLAVE_ID 1
#define VERSION  2 
#define OLED_128X32_ADDRESS 0x3C
#define RS485_BAUDRATE 9600

#define PACKET_STX 0x02
#define PACKET_ETX 0xFF

// #define LED_BUILTIN D7

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

#define RS485 Serial1

typedef struct __attribute__ ((packed)) solPacket
{
  byte STX;
  byte REQUEST;
  byte ESP_STATE;
  byte LED_BRIGHTNESS;
  byte SOLENOID_STATE;
  byte PHOTO_SWITCH;
  byte PHOTO_TRIGGER;
  byte ETX;
}solenoid_t;

typedef union
{
  solenoid_t sol;
  uint8_t data[sizeof(solenoid_t)];

}sol_packet_t;

sol_packet_t recvPacket;
sol_packet_t sendPacket;
solenoid_t *recvSolenoid = &recvPacket.sol;
solenoid_t *sendSolenoid = &sendPacket.sol;


typedef enum
{
  LED_TURN_OFF  = 0xF0,
  LED_TURN_ON   = 0xF1,
  SOL_PUSH      = 0xF2,
  SOL_RELEASE   = 0xF3,
  PHOTO_START   = 0xF4,
  PHOTO_STOP    = 0xF5,
  PHOTO_TRIG    = 0xF6,
  PHOTO_READING = 0xF7,
  REQUEST_DATA  = 0xF8,
  RETURN_DATA   = 0xF9
}SolREQUEST;

typedef enum
{
  ESP_OTA_MODE = 0x55,
  ESP_NORMAL_MODE = 0xAA,
  PHOTO_OFF = 0x00,
  PHOTO_ON = 0x01,
  LED_OFF = 0x00,
  LED_ON = 0x01,
}SolState;


uint64_t breathingTime = 0;
bool breathingDirection = false;
int breathingValue = 0;

bool oledState = false;
uint64_t lastTime = 0;
uint64_t count = 0;

int solenoidState = 0;

uint64_t lightTime = 0;
bool lightState = false;
bool lightFlag = false; 
uint8_t lightValue = 0;

int otaStatus = 0;
int otaStatusCount = 0;

uint16_t lastRelayValue = 0;
bool relayState = false;
uint32_t rs485Time = 0;
uint32_t solenoidTime = 0;
uint32_t relayTime = 0;
bool relayToggle = false;


bool photoFlag = false;
int photoTrigger = 0;
uint64_t photoTime = 0;
int photoCount = 0;


// EspSoftwareSerial::UART RS485;

MyOTA* myOTA = new MyOTA();
MyLittleFS* myFS = new MyLittleFS();
Button myBtn(BUTTON_PIN, false, 20);  // 0 -> HIGH , Pull-up ->Invert (true) 
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE);




void receivePacket();
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
  RS485.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  canInit();
  sendSolenoid->STX = PACKET_STX;
  sendSolenoid->ETX = PACKET_ETX;
  // RS485.begin(RS485_BAUDRATE, EspSoftwareSerial::SWSERIAL_8N1, RS485_RX, RS485_TX);


  initPinout();
  initOLED();

  // delay(5000);

  // Find button Status
  while (true)
  { 
    myBtn.read();
    if(myBtn.pressedFor(3000,5000))
    {
      otaStatus = 1;
      sendSolenoid->ESP_STATE = ESP_OTA_MODE;

      Serial.printf("STATUS : %d ------------------------\r\n", otaStatus);
      break;
    }
    else if (myBtn.wasReleased()) 
    {
      otaStatus = 0;
      sendSolenoid->ESP_STATE = ESP_NORMAL_MODE;
   
      Serial.printf("STATUS : %d ------------------------\r\n", otaStatus);
      break;
    }
    else if(myBtn.isReleased()) // Current Mode
    {
      otaStatus = 0;
      sendSolenoid->ESP_STATE = ESP_NORMAL_MODE;
      
      Serial.printf("STATUS : released %d ------------------------\r\n", otaStatus);
      break;
    }
  }
  canSend(0x257, 8, 100, sendPacket.data); // Send CAN Message

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

  
  
  // Serial.println("Done");
}

void loop() {

  
 

  if(millis() - lastTime > 1000)
  {
    lastTime = millis();
    count++;

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

  
  if(otaStatus)
  {
    myOTA->loop();
  }
  else
  {
    // myModbus->pollModbus();
    // receivePacket();
    // uint8_t myData[8] = {0,1, 2, 3, 4, 5, 6, 7};
    
    CANMessage receivedFrame = canReceive(); // Receive CAN Message 

    if(receivedFrame.id != 0)
    {
      Serial.printf("ID: 0x%03X, Data: ", receivedFrame.id);
      for (int i = 0; i < receivedFrame.len; i++)
      {
        Serial.printf("0x%02X ", receivedFrame.data[i]);
      }
      Serial.println();

      memcpy(recvPacket.data, receivedFrame.data, sizeof(solenoid_t));
      // Serial.printf("STX 0x%02X\r\n", recvSolenoid->STX);
      // Serial.printf("ETX 0x%02X\r\n", recvSolenoid->ETX);

      if(recvSolenoid->STX == PACKET_STX && recvSolenoid->ETX == PACKET_ETX)
      {
        Serial.printf("0x%02X\r\n", recvSolenoid->REQUEST);
        Serial.printf("0x%02X\r\n", recvSolenoid->ESP_STATE);
        Serial.printf("0x%02X\r\n", recvSolenoid->LED_BRIGHTNESS);
        Serial.printf("0x%02X\r\n", recvSolenoid->SOLENOID_STATE);
        Serial.printf("0x%02X\r\n", recvSolenoid->PHOTO_SWITCH);
        Serial.printf("0x%02X\r\n", recvSolenoid->PHOTO_TRIGGER);
        
        switch (recvSolenoid->REQUEST)
        {
        case LED_TURN_ON:   // 0xF1
          lightFlag = true;
          lightValue = recvSolenoid->LED_BRIGHTNESS;
          Serial.printf("LED Value = %d\r\n", lightValue);
          break;
        case LED_TURN_OFF:  // 0xF0
          lightFlag = true;
          lightValue = 0;
          Serial.println("LED Turn Off");
          break;

        case SOL_PUSH:   // 0xF2
          Serial.println("RX : Push");
          solenoidState = SOL_PUSH;

          break;
        case SOL_RELEASE: // 0xF3
          Serial.println("RX : Release");
          solenoidState = SOL_RELEASE;

          break;
        case PHOTO_START:   // 0xF4
          Serial.println("RX : Photo Start");
          photoFlag = true;
          break;

        case PHOTO_STOP:  // 0xF5
          Serial.println("RX : Photo Stop");
          photoFlag = false;
          break;
        
        case REQUEST_DATA:
          Serial.println("Return Data");
          // sendSolPacket(RETURN_DATA);

        default:
          break;
        }

      }

      
    }

    
    controlSolenoid();
    setLED();
    photoSensing();
    pressKey(); // Debug
    canSend (0x257, 8, 100, sendPacket.data); // Send CAN Message
  }
  localSwitch();
  breathe(5);
  // rs485Command();
}


void sendSolPacket(byte myRequest)
{
  sendSolenoid->REQUEST = myRequest;
  sendSolenoid->LED_BRIGHTNESS = lightValue;
  sendSolenoid->SOLENOID_STATE = solenoidState;
  sendSolenoid->PHOTO_TRIGGER = photoTrigger;

  RS485.write((uint8_t*)&sendPacket, sizeof(sendPacket));
  // Serial.println("Send Packet !!");
}

// void receivePacket()
// {
//   if(millis() - rs485Time >= 1)
//   {
//     if(RS485.available())
//     {
//       RS485.readBytes(recv, structSize);
//       for (int i = 0; i < structSize; i++)
//       {
//         Serial.printf(" 0x%02X | ", recv[i]);
//       }
//       Serial.println();
      
//       receivedPacket.REQUEST = recv[1];
//       receivedPacket.LED_BRIGHTNESS = recv[2];
//       // receivedPacket.SOLENOID_STATE = recv[3];

//       if(recv[0] == PACKET_STX && recv[structSize - 1] == PACKET_ETX)
//       {
//         switch (receivedPacket.REQUEST)
//         {
//         case LED_TURN_ON:
//           lightState = true;
//           lightValue = receivedPacket.LED_BRIGHTNESS;
//           Serial.printf("LED Value = %d\r\n", lightValue);
//           break;

//         case SOL_PUSH:
//           Serial.println("RX : Push");
//           solenoidState = SOL_PUSH;

//           break;
//         case SOL_RELEASE:
//           Serial.println("RX : Release");
//           solenoidState = SOL_RELEASE;

//           break;
//         case PHOTO_START:
//           Serial.println("RX : Photo Start");
//           photoFlag = true;
//           break;

//         case PHOTO_STOP:
//           Serial.println("RX : Photo Stop");
//           photoFlag = false;
//           break;
        
//         case REQUEST_DATA:
//           Serial.println("Return Data");
//           sendSolPacket(RETURN_DATA);

//         default:
//           break;
//         }

//       }

      
//     }
//     rs485Time = millis();
//   }
  
// }

void initPinout()
{
  // Serial.println("Set Pinout");
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
    // Serial.println("Failed to obtain time");
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
        Serial.printf("1-> ");
        setRelay1();
        break;         
      }
  
      case '2':
      {
        Serial.printf("2-> ");
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
  // Serial.printf("Button State : %d\r\n", myBtn.read());

  if(myBtn.pressedFor(3000,5000))
  {
    Serial.printf("BUTTON PressedFor 3000\r\n");
    // ESP.restart(); // 아직 OTA 동작 X
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
        solenoidState = SOL_PUSH;
      }
      else
      {      
        solenoidState = SOL_RELEASE;
      }
      // relayToggle = true;

      // Serial.println("Button -> Save Sol and Led");
      Serial.printf("Sol : 0x%02X | LED : %3d \r\n", solenoidState, lightValue);
      myFS->saveSol(LittleFS, solenoidState, lightValue);
      setOLED();
      // Serial.printf("%s\r\n", msg);
      
  }
  
}


void setRelay1()
{
  relayState = true;
  solenoidState = SOL_PUSH;
  // myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0),10, 1 );
  // Serial.printf("Push Solenoid\r\n");
}

void setRelay2()
{
  relayState = false;
  solenoidState = SOL_RELEASE;
  
  // myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255),10, 1 );
  // Serial.printf("Release Solenoid\r\n");
}

void controlSolenoid()
{
  if(solenoidState != lastRelayValue)
  {
    lastRelayValue = solenoidState;
    
    if(solenoidState == SOL_PUSH || solenoidState == SOL_RELEASE)
    {
      relayToggle = true;
    }
    
    // myModbus->holdingRegisters[1] = 0x00; // Execute Once
    myFS->saveSol(LittleFS, solenoidState, lightValue);
  }


  if(relayToggle == true && millis() - relayTime >= 50)
  {
    relayTime = millis();
    if(solenoidState == SOL_PUSH)
    {
      digitalWrite(RELAY_1_PIN, HIGH);
      Serial.println("Sol - 1 ");
      relayState = true;
    }
    else if(solenoidState == SOL_RELEASE)
    {
      digitalWrite(RELAY_2_PIN, HIGH);
      Serial.println("Sol - 2 ");
      relayState = false;
    }
    sendSolenoid->SOLENOID_STATE = solenoidState;
    relayToggle = false;
    
  }
  else if(relayToggle == false && millis() - relayTime >= 50)
  {
    relayTime = millis();
    digitalWrite(RELAY_1_PIN, LOW);
    digitalWrite(RELAY_2_PIN, LOW);
    // Serial.println("Sol toggle OFF");
  }

  // setOLED();
}

void setLED()
{
  if(lightFlag)
  {
    
    if(millis() - lightTime > 100)
    {
      ledcWrite(ledChannel2 , lightValue);
      myFS->saveSol(LittleFS, solenoidState, lightValue);
      sendSolenoid->LED_BRIGHTNESS = lightValue;
      setOLED();

      lightFlag = false;  // JUST ONCE
      lightTime = millis();
    }
  }
}

void photoSensing()
{
  if(photoFlag)
  {
    // Serial.printf("Start photo: %d\r\n", photoFlag);
    if(millis() - photoTime >= 50)
    {

      photoTime = millis();
      int photoValue = digitalRead(PHOTO_SENSOR_PIN);
      sendSolenoid->PHOTO_SWITCH = PHOTO_ON;
      sendSolenoid->PHOTO_TRIGGER = photoValue;
      
      

      Serial.printf("PHOTO : %d\r\n", photoValue);
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
        sendSolenoid->PHOTO_TRIGGER = PHOTO_TRIG;
        Serial.printf("해당 위치에 도달하였습니다!\r\n");
        // sendSolenoid->PHOTO_SWITCH = PHOTO_OFF;
      }
      else
      {
        photoTrigger = PHOTO_READING;
        // sendSolPacket(PHOTO_READING);
      }      
    }
  }
  else
  {
    sendSolenoid->PHOTO_SWITCH = PHOTO_OFF;
    photoCount = 0;
  }
}


void rs485Command()
{
  if(millis() - rs485Time >= 1)
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
  u8g2.printf("SOL [0x%02d]|LED [%3d]|", solenoidState, lightValue);
  u8g2.sendBuffer();

}