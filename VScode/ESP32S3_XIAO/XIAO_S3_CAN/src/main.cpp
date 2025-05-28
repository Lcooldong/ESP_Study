
#include <Arduino.h>
#include <ACAN_ESP32.h>
#include <core_version.h>


const int ledChannel1 = 0;
const int ledFreq = 5000;
const int ledResolution = 8;
uint32_t count = 0;

ACAN_ESP32 & myCAN = ACAN_ESP32::can;
static const uint32_t DESIRED_BIT_RATE = 500UL * 1000UL;

void setup() {
  Serial.begin(115200);
  ledcSetup(ledChannel1, ledFreq, ledResolution);
  ledcAttachPin(LED_BUILTIN, ledChannel1); 
  while (!Serial){}
  delay(5000); 

  esp_chip_info_t chip_info ;
  esp_chip_info (&chip_info) ;
  Serial.print ("ESP32 Arduino Release: ") ;
  Serial.println (ARDUINO_ESP32_RELEASE) ;
  Serial.print ("ESP32 Chip Revision: ") ;
  Serial.println (chip_info.revision) ;
  Serial.print ("ESP32 SDK: ") ;
  Serial.println (ESP.getSdkVersion ()) ;
  Serial.print ("ESP32 Flash: ") ;
  uint32_t size_flash_chip ;
  esp_flash_get_size (NULL, &size_flash_chip) ;
  Serial.print (size_flash_chip / (1024 * 1024)) ;
  Serial.print (" MB ") ;
  Serial.println (((chip_info.features & CHIP_FEATURE_EMB_FLASH) != 0) ? "(embeded)" : "(external)") ;
  Serial.print ("APB CLOCK: ") ;
  Serial.print (APB_CLK_FREQ / 1000000) ;
  Serial.println (" MHz") ;
  
 
  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE);
  settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode;
  // settings.mRequestedCANMode = ACAN_ESP32_Settings::LoopBackMode;
  settings.mTxPin = GPIO_NUM_2; // Set your TX pin  -> D1
  settings.mRxPin = GPIO_NUM_1; // Set your RX pin  -> D0
  // settings.mBitRatePrescaler = 10; 
  // settings.mTimeSegment1 = 13; // Set Time Segment 1
  // settings.mTimeSegment2 = 2; // Set Time Segment 2

  const uint32_t errorCode = myCAN.begin (settings) ;
  if (errorCode == 0) {
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
  }else {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

}

static uint32_t gBlinkLedDate = 0 ;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;
static const uint32_t MESSAGE_COUNT = 1 ;

void loop() {
  CANMessage frame ;
  if (gBlinkLedDate < millis ()) {
    gBlinkLedDate += 100 ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
    Serial.print ("Sent: ") ;
    Serial.print (gSentFrameCount) ;
    Serial.print (" ") ;
    Serial.print ("Receive: ") ;
    Serial.print (gReceivedFrameCount) ;
    Serial.print (" ") ;
    Serial.print (" STATUS 0x") ;
    Serial.print (myCAN.TWAI_STATUS_REG (), HEX) ;
    Serial.print (" RXERR ") ;
    Serial.print (myCAN.TWAI_RX_ERR_CNT_REG ()) ;
    Serial.print (" TXERR ") ;
    Serial.println (myCAN.TWAI_TX_ERR_CNT_REG ()) ;


    count++;
    frame.id = 0x258;
    frame.len = 8; // Set the length of data

    for (int i = 0; i < frame.len; i++)
    {
      frame.data[i] = i + count; // Initialize data to zero
    }
    
    // frame.data[0] = 0x01; // Example data 

    const bool ok = myCAN.tryToSend (frame) ;
    if (ok) {
      gSentFrameCount += 1 ;
    }
  }
  uint32_t sendCount = 0;
  uint32_t receiveCount = 0;
  uint32_t statusPrintDate = millis();
  
  


  while (myCAN.receive (frame)) {
    gReceivedFrameCount += 1 ;
  }



}

