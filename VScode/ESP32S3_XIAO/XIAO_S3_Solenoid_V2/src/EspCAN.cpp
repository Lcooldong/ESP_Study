#include "EspCAN.h"


void canInit()
{
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



bool canSend(int id, uint8_t len, uint32_t interval, uint8_t *data)
{
  CANMessage frame ;
  if (gCANSendDate < millis ()) 
  {
    gCANSendDate += interval ;
    // Serial.print ("Sent: ") ;
    // Serial.print (gSentFrameCount) ;
    // Serial.print (" ") ;
    // Serial.print ("Receive: ") ;
    // Serial.print (gReceivedFrameCount) ;
    // Serial.print (" ") ;
    // Serial.print (" STATUS 0x") ;
    // Serial.print (myCAN.TWAI_STATUS_REG (), HEX) ;
    // Serial.print (" RXERR ") ;
    // Serial.print (myCAN.TWAI_RX_ERR_CNT_REG ()) ;
    // Serial.print (" TXERR ") ;
    // Serial.println (myCAN.TWAI_TX_ERR_CNT_REG ()) ;


    canCount++;
    frame.id = id;
    frame.len = len; // Set the length of data

    if(data != NULL)
    {
      memcpy(frame.data, data, len); // Copy data to frame
    }


    // for (int i = 0; i < frame.len; i++)
    // {
    //   frame.data[i] = data[i]; // Initialize data to zero
    // }
    
    // frame.data[0] = 0x01; // Example data 

    const bool ok = myCAN.tryToSend (frame) ;
    if (ok) {
      gSentFrameCount += 1 ;
    }

    return ok;
  }

  return false;
}

CANMessage canReceive()
{
  CANMessage frame ;
  while (myCAN.receive (frame)) 
  {
    gReceivedFrameCount += 1 ;

    Serial.print ("Received [") ;
    Serial.print (gReceivedFrameCount) ;
    Serial.print ("]=>") ;
    // Serial.print (" ID: ") ;
    // Serial.print (frame.id, HEX) ;
    // Serial.print (" Data: ");
    
    // if(frame.len > 0)
    // {
    //   for (int i = 0; i < frame.len; i++)
    //   {
    //     Serial.printf("0x%02X ", frame.data[i]);
    //   }
    //   Serial.println();
    // }
  }

  return frame;
}