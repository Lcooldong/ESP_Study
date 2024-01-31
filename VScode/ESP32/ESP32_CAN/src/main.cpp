#ifndef ARDUINO_ARCH_ESP32
  #error "Select an ESP32 board"
#endif

#include <Arduino.h>
#include <ACAN_ESP32.h>
#include <core_version.h> // For ARDUINO_ESP32_RELEASE
//#include <SoftwareSerial.h>

#define LED_BUILTIN GPIO_NUM_2
#define CAN_RX      GPIO_NUM_16
#define CAN_TX      GPIO_NUM_17

//static const uint32_t DESIRED_BIT_RATE = 1000UL * 1000UL ; // 1 Mb/s
static const uint32_t DESIRED_BIT_RATE = 125UL * 1000UL ;
uint64_t lastTime = 0;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  
  delay(100);
  while(!Serial)
  {
    delay(50);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  Serial.println("================================");
  esp_chip_info_t chip_info ;
  esp_chip_info (&chip_info) ;
  Serial.print ("ESP32 Arduino Release: ") ;
  Serial.println (ARDUINO_ESP32_RELEASE) ;
  Serial.print ("ESP32 Chip Revision: ") ;
  Serial.println (chip_info.revision) ;
  Serial.print ("ESP32 SDK: ") ;
  Serial.println (ESP.getSdkVersion ()) ;
  Serial.print ("ESP32 Flash: ") ;
  Serial.print (spi_flash_get_chip_size () / (1024 * 1024)) ;
  Serial.print (" MB ") ;
  Serial.println (((chip_info.features & CHIP_FEATURE_EMB_FLASH) != 0) ? "(embeded)" : "(external)") ;
  Serial.print ("APB CLOCK: ") ;
  Serial.print (APB_CLK_FREQ) ;
  Serial.println (" Hz") ;
  
  Serial.println("---------ESP32 CAN Start---------");

  // Connect CAN 
  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE);
//  settings.mRequestedCANMode = ACAN_ESP32_Settings::LoopBackMode;
  
  settings.mRxPin = CAN_RX;
  settings.mTxPin = CAN_TX;
  
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::acceptStandardFrames();
  
  const uint32_t errorCode = ACAN_ESP32::can.begin(settings, filter);


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

  digitalWrite(LED_BUILTIN, HIGH);
}

static uint32_t gBlinkLedDate = 0 ;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;

void loop() {
  //  CANMessage frame;

  // if(millis() - lastTime > 1000){
  //   lastTime = millis();
    
  //   //1000ms마다 이부분이 반복실행된다!

  //   //호출하고자하는 슬레이브의 ID
  //   //standard일때(11bit)
  //   //0x000(0) ~ 7FF(2,047)
  //   //extended일때(29bit)
  //   //0x000(0) ~ 0x1FFFFFFF(536,870,911)
  //   frame.id = 0x123;
  //   frame.rtr = 0; //확실히 이게 무슨 기능을 하는지는 모르겠음!
  //   frame.len = 8; //내가 보낼 데이터의 길이

  //   frame.data[0] = 'A';
  //   frame.data[1] = 'B';
  //   frame.data[2] = 'C';
  //   frame.data[3] = 'D';
  //   frame.data[4] = 'E';
  //   frame.data[5] = 'F';
  //   frame.data[6] = 'G';
  //   frame.data[7] = 'H';

  //   //전송
  //   if (ACAN_ESP32::can.tryToSend(frame)) {
  //     //성공 
  //     Serial.printf("%d : 전송에 성공했습니다\r\n", gSentFrameCount++);
  //     digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN));
  //   }   
  // }



  CANMessage frame;

  //캔통신으로 수신된 데이터가 있는가?
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

  // CANMessage frame ;
  // if (gBlinkLedDate < millis ()) {
  //   gBlinkLedDate += 1000 ;
  //   digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
  //   Serial.print ("Sent: ") ;
  //   Serial.print (gSentFrameCount) ;
  //   Serial.print (" ") ;
  //   Serial.print ("Receive: ") ;
  //   Serial.print (gReceivedFrameCount) ;
  //   Serial.print (" ") ;
  //   Serial.print (" STATUS 0x") ;
  //   Serial.print (TWAI_STATUS_REG, HEX) ;
  //   Serial.print (" RXERR ") ;
  //   Serial.print (TWAI_RX_ERR_CNT_REG) ;
  //   Serial.print (" TXERR ") ;
  //   Serial.println (TWAI_TX_ERR_CNT_REG) ;
  //   const bool ok = ACAN_ESP32::can.tryToSend (frame) ;
  //   if (ok) {
  //     gSentFrameCount += 1 ;
  //   }
  // }
  // while (ACAN_ESP32::can.receive (frame)) {
  //   gReceivedFrameCount += 1 ;
  // }    
}

