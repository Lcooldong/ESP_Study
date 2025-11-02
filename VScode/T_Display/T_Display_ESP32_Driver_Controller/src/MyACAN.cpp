#include "MyACAN.h"

bool can_init(int rx_pin, int tx_pin)
{
  bool ret = false;

  my_rx_pin = rx_pin;
  my_tx_pin = tx_pin;

  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE) ;
  settings.mRxPin = (gpio_num_t)rx_pin; 
  settings.mTxPin = (gpio_num_t)tx_pin ;
  settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode ;
  const uint32_t errorCode = my_can.begin (settings) ;

  if (errorCode == 0) {
    Serial.printf("APB Frequency     : %d MHz\r\n", getApbFrequency()/1000000);
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

    ret = true;
  }else {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

 return ret;
}



bool can_send(int id, uint8_t *data, size_t len)
{
  static uint32_t sent_frame_count = 0; // Static variable to keep track of sent frames
  CANMessage frame ;
  
  frame.id = id ;
  frame.len = len ;
  if(data != NULL && len > 0) {
    memcpy(frame.data, data, len); // Copy data to frame
  }
  
  const bool ok = my_can.tryToSend (frame) ;
  if (ok) {
    sent_frame_count++;
    Serial.printf("[%d]CAN frame sent: ID=0x%X, Data=", sent_frame_count, frame.id);
    for (size_t i = 0; i < len; i++) {
      Serial.printf("%02X ", frame.data[i]);
    }
    Serial.println();
    
  } else {
    Serial.printf("Failed to send CAN frame\r\n");
    if(my_can.driverTransmitBufferSize() < my_can.driverTransmitBufferPeakCount())
    {
      Serial.printf("ESP Restart for can reconnect\r\n");
      ESP.restart();
    }
  }


  return ok; // Return true if the frame was sent successfully
}




CANMessage can_receive()
{
  
  static uint32_t received_frame_count = 0; // Static variable to keep track of received frames
  CANMessage frame;

  if(my_can.available()) {
    my_can.receive(frame);
    
    received_frame_count++;
    Serial.printf("[%d]CAN frame received: ID=0x%X, Data=", received_frame_count, frame.id);
    for (size_t i = 0; i < frame.len; i++) {
      Serial.printf("%02X ", frame.data[i]);
    }
    Serial.println();
    
  } 
  return frame; // Return the received frame
}

// only for receive can't reconnect
bool can_connect()
{
  bool ret = false;
  my_can.kNotInResetModeInConfiguration;
  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE);
  settings.mRxPin = (gpio_num_t)my_rx_pin ; 
  settings.mTxPin = (gpio_num_t)my_tx_pin ;
  settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode ;

  const uint32_t errorCode = my_can.begin (settings) ;
  // const uint32_t errorCode = ACAN_ESP32::can.begin (settings) ;
  delay(50);
  if(errorCode == 0) {
    Serial.println("CAN connection established successfully!");
    ret = true;
  } else {
    Serial.printf("CAN connection failed with error code: 0x%X\r\n", errorCode);
  }
  

  return  ret;
}