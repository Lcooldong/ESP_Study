#include <Arduino.h>
#include <HardwareSerial.h>
#include <ACAN_ESP32.h>
#include "neopixel.h"
#include "Button.h"

#define HWSERIAL Serial1
#define SEEED_XIAO_C3
//#define SEEED_XIAO_S3

// C3
#ifdef SEEED_XIAO_C3

const int relay_1_Pin = GPIO_NUM_2;
const int relay_2_Pin = GPIO_NUM_3;
const int button_Pin = GPIO_NUM_4;
const int led_Pin = GPIO_NUM_5;      // GPIO4
const int photo_Pin = GPIO_NUM_8;
gpio_num_t can_RX = GPIO_NUM_6;
gpio_num_t can_TX = GPIO_NUM_7;

#else if SEEED_XIAO_S3 

const int relay_1_Pin = GPIO_NUM_2;
const int relay_2_Pin = GPIO_NUM_3;
const int button_Pin = GPIO_NUM_4;
const int led_Pin = GPIO_NUM_5;      // GPIO4
const int photo_Pin = GPIO_NUM_8;
gpio_num_t can_RX = GPIO_NUM_6;
gpio_num_t can_TX = GPIO_NUM_7;

#endif

static const uint32_t DESIRED_BIT_RATE = 500UL * 1000UL ;
static uint32_t gBlinkLedDate = 0 ;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;

bool relayState = false;
uint8_t led_Value = 0;

int photoState = 0;
int photoFlag = 0;
uint64_t photoTime = 0;
int photoCount = 0;

int flow = 0;
uint64_t flowTime = 0;

MyNeopixel* myNeopixel = new MyNeopixel();
Button myBtn(button_Pin, 0, 50);


void setRelay1();
void setRelay2();
void receiveCAN();
void sendCAN(uint32_t _id, uint8_t _array[8], bool _remote);
void testCommand();
void photoSensing();
void localSwitch();
int lightState = 0;

void setup() {
  Serial.begin(115200);
  
  HWSERIAL.begin(115200, SERIAL_8E1, 10, 9);
  
   // RS485
  pinMode(relay_1_Pin, OUTPUT);
  pinMode(relay_2_Pin, OUTPUT);
  pinMode(button_Pin, INPUT);   // Hardware Pull-Up
  pinMode(photo_Pin, INPUT);

  digitalWrite(relay_1_Pin, LOW);
  digitalWrite(relay_2_Pin, LOW);

  ledcAttachPin(led_Pin, 0);
  ledcSetup(0, 5000, 8);
  ledcWrite(0, 0);

  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE);
  settings.mRequestedCANMode = ACAN_ESP32_Settings::LoopBackMode;
  
  settings.mRxPin = can_RX;
  settings.mTxPin = can_TX;
  
  // const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::acceptStandardFrames();
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::dualStandardFilter(
    ACAN_ESP32_Filter::data, 0x123, 0x404,  // id Filter, DontCareMask
    ACAN_ESP32_Filter::remote, 0x456, 0x022
  );

  const uint32_t errorCode = ACAN_ESP32::can.begin(settings, filter);
  ACAN_ESP32 myCAN();

  if( 0 == errorCode)
  {
    Serial.print ("Bit Rate prescaler: ") ;
    Serial.println (settings.mBitRatePrescaler) ;
    Serial.print ("Time Segment 1:     ") ;
    Serial.println (settings.mTimeSegment1) ;
    Serial.print ("Time Segment 2:     ") ;
    Serial.println (settings.mTimeSegment2) ;
    Serial.print ("RJW:                ") ;
    Serial.println (settings.mRJW) ;
    Serial.print ("Triple Sampling:    ") ;
    Serial.println (settings.mTripleSampling ? "yes" : "no") ;
    Serial.print ("Actual bit rate:    ") ;
    Serial.print (settings.actualBitRate ()) ;
    Serial.println (" bit/s") ;
    Serial.print ("Exact bit rate ?    ") ;
    Serial.println (settings.exactBitRate () ? "yes" : "no") ;
    Serial.print ("Distance            ") ;
    Serial.print (settings.ppmFromDesiredBitRate ()) ;
    Serial.println (" ppm") ;
    Serial.print ("Sample point:       ") ;
    Serial.print (settings.samplePointFromBitStart ()) ;
    Serial.println ("%") ;
    Serial.println ("Configuration OK!");
  }
  else
  {
    Serial.printf("Error CAN : %0x", errorCode);
  }



  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0),10, 1 );
}



void loop() {
  receiveCAN();

  if(millis() - flowTime > 1000)
  {
    CANMessage frameSend;

    flowTime = millis();
    flow++;
    Serial.printf("%d\r\n", flow);
    HWSERIAL.printf("%d\r\n", flow);

    frameSend.id = 0x123;
    frameSend.rtr = 0; //확실히 이게 무슨 기능을 하는지는 모르겠음!
    frameSend.len = 8; //내가 보낼 데이터의 길이

    // for (int i = 0; i < 8; i++)
    // {
      
    // }
    frameSend.data64 = flow;
  
    // frameSend.data[0] = 'A';
    // frameSend.data[1] = 'B';
    // frameSend.data[2] = 'C';
    // frameSend.data[3] = 'D';
    // frameSend.data[4] = 'E';
    // frameSend.data[5] = 'F';
    // frameSend.data[6] = 'G';
    // frameSend.data[7] = 'H';

    //전송 16번
    if (ACAN_ESP32::can.tryToSend(frameSend)) {
      //성공 
      Serial.printf("[%d] ID:%X \r\n", gSentFrameCount++, frameSend.id);
    }
    // else
    // {
    //   ACAN_ESP32::can.recoverFromBusOff();
    //   Serial.printf("CAN OUT  \r\n" );
    //   frameSend.data[0] = '1';
    //   frameSend.data[1] = '2';
    //   frameSend.data[2] = '3';
    //   frameSend.data[3] = '4';
    //   frameSend.data[4] = '5';
    //   frameSend.data[5] = '6';
    //   frameSend.data[6] = '7';
    //   frameSend.data[7] = '8';
    //   ACAN_ESP32::can.tryToSend(frameSend);
    // }

  }
  
  // if(!digitalRead(button_Pin))
  // {
  //   if(relayState)
  //   {
  //     setRelay1();
  //   } 
  //   else
  //   {
  //     setRelay2();
  //   }
  //   relayState = !relayState;

  //   while (!digitalRead(button_Pin))
  //   {
  //     Serial.printf("Button Clicking\r\n");
  //     delay(100);
  //   }

  //   delay(50);
  // }
  testCommand();

  photoSensing();

  localSwitch();
  // for (int i = 0; i < 255; i++)
  // {
  //   ledcWrite(0, i);
  //   delay(5);
  // }
  // for (int i = 255; i >0; i--)
  // {
  //   ledcWrite(0, i);
  //   delay(5);
  // }
  
  
}


void setRelay1()
{
  digitalWrite(relay_1_Pin, LOW);
  delay(100);
  digitalWrite(relay_1_Pin, HIGH);
  delay(100);
  digitalWrite(relay_1_Pin, LOW);
  delay(100);
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0),10, 1 );
  Serial.printf("Push Solenoid\r\n");
}

void setRelay2()
{
  digitalWrite(relay_2_Pin, LOW);
  delay(100);
  digitalWrite(relay_2_Pin, HIGH);
  delay(100);
  digitalWrite(relay_2_Pin, LOW);
  delay(100);
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255),10, 1 );
  Serial.printf("Release Solenoid\r\n");
}

void receiveCAN()
{
  CANMessage frame;

  if(ACAN_ESP32::can.receive(frame)) {
    //frame.ext : ID종류가 29bit인가? 아니면 11bit인가
    //frame.rtr : 리모트인가? 아니면 데이터인가
    //frame.id : 아이디
    //frame.len : 수신데이터 길이
    //frame.data : 수신데이터(배열)
    if(frame.ext){
      Serial.println("EXTENDED ID입니다!");
    }else{
      Serial.println("STANDARD ID입니다!");
    }

    if(frame.rtr){
      Serial.println("RTR : REMOTE!");
    }else{
      Serial.println("RTR : DATA!");
    }

    Serial.print("수신받은 ID : ");
    Serial.println(frame.id,HEX);

    Serial.print("데이터길이 : ");
    Serial.println(frame.len);

    //결과를 16진수로출력
    Serial.print("데이터를 16진수로 표현 : ");
    for(int i = 0;i<frame.len;i++){
      Serial.print(frame.data[i],HEX);
      Serial.print(", ");
    }
    Serial.println();

    //결과를 아스키코드로출력
    Serial.print("데이터를 16진수로 표현 : ");
    for(int i = 0;i<frame.len;i++){
      Serial.print((char)frame.data[i]);
      Serial.print(", ");
    }
    Serial.println();
  }  
}


void sendCAN(uint32_t _id, uint8_t _array[8], bool _remote = false)
{
  CANMessage frameToSend;



    frameToSend.id = 0x123;
    frameToSend.rtr = _remote; //- DataFrame :전송할 데이터/ RemoteFrame : 원하는 메세지 전송 요청
    frameToSend.len = 8; //내가 보낼 데이터의 길이

  
    for (int i = 0; i < frameToSend.len; i++)
    {
      frameToSend.data[i] = _array[i];
    }
    

    //전송 16번
    if (ACAN_ESP32::can.tryToSend(frameToSend)) {
      //성공 
      Serial.printf("[%d] ID:%X \r\n", gSentFrameCount++, frameToSend.id);
    }
}

void testCommand()
{
  int ch = Serial.read();
  
  if(ch != -1)
  {
    // Serial.printf("READ : %d\r\n", ch);
    switch (ch)
    {
    case 'a':
      
      if (led_Value >= 255)
      {
        led_Value = 255;
      }
      else
      {
        led_Value++;
      }
      Serial.printf("Value : %d\r\n", led_Value);
      HWSERIAL.printf("LED : %d\r\n", led_Value);
      ledcWrite(0, led_Value);
      delay(10);
      break;
    case 'b':
      
      if (led_Value <= 0)
      {
        led_Value = 0;
      }
      else
      {
        led_Value--;
      }
      Serial.printf("Value : %d\r\n", led_Value);
      HWSERIAL.printf("LED : %d\r\n", led_Value);
      ledcWrite(0, led_Value);
      delay(10);
      break;  
    case 'c':
      setRelay1();
      break;

    case 'd':
      setRelay2();
      break;

    case 'p':
      photoFlag = !photoFlag;
      Serial.printf("Photo Flag : %d\r\n", photoFlag); 
      delay(500);
      
      break;

    default:      
      break;
    }
 }

}

void photoSensing()
{
  if(photoFlag)
  {
    if(millis() - photoTime > 10)
    {
      photoTime = millis();
      int photoValue = digitalRead(photo_Pin);
      Serial.printf("PHOTO : %d\r\n", photoValue);
      HWSERIAL.printf("PHOTO : %d\r\n", photoValue);
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