#include <Arduino.h>
#include "M5Atom.h"
#include "neopixel.h"
//#include <PN532/PN532_HSU/PN532_HSU.h>
#include <NfcAdapter.h>
#include <PN532/PN532/PN532.h>

#include <PN532/PN532_SWHSU/PN532_SWHSU.h>
#include <PN532/PN532_HSU/PN532_HSU.h>
#include <SoftwareSerial.h>

const int RX1_PIN = GPIO_NUM_22;
const int TX1_PIN =  GPIO_NUM_19;

uint64_t NFC_READ_TIME = 0;

HardwareSerial HWSerial(Serial1);
PN532_HSU pn532hsu(Serial1);
NfcAdapter nfc(pn532hsu);
// SoftwareSerial SWSerial(RX1_PIN, TX1_PIN);
// PN532_SWHSU pn532swhsu(SWSerial);
// NfcAdapter nfc = NfcAdapter(pn532swhsu);

long count = 0;
MyNeopixel* myNeopixel = new MyNeopixel();

void setup() {
  M5.begin(true, false, false);
  // Serial.begin(115200);
  HWSerial.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN); 

  

  if (!HWSerial) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
    M5.update();
    while (1) { // Don't continue with invalid configuration
      delay (1000);
    }
  }
  else
  {
    Serial.printf("HWSerial Done\r\n");
  }


  // nfc.begin();
  myNeopixel->InitNeopixel();
  
  Serial.println("Start Atom");
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 255), 50, 50);

  delay(1000);
}

void loop() {
  if(M5.Btn.wasPressed())
  {
    Serial.printf("Count : %d\r\n", count++);
    HWSerial.printf("Count : %d\r\n", count++);
    
    if (nfc.tagPresent()) {
        NdefMessage message = NdefMessage();
        message.addTextRecord("Hello, Arduino!");
        message.addUriRecord("http://arduino.cc");
        message.addTextRecord("Goodbye, Arduino!");
        boolean success = nfc.write(message);
        if (success) {
            Serial.println("Success. Try reading this tag with your phone.");
        } else {
            Serial.println("Write failed");
        }
    }
    delay(50);
  }
  
  // M5.Btn.read();
  if (millis() - NFC_READ_TIME > 2000)
  {
    NFC_READ_TIME = millis();
    Serial.println("\nScan a NFC tag\n");
    if (nfc.tagPresent()) {
        NfcTag tag = nfc.read();
        tag.print();
    }
  }
  
  M5.update();
}



