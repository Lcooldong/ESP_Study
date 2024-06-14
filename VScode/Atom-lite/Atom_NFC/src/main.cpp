#include <Arduino.h>
#include "M5Atom.h"
#include "neopixel.h"




#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   (19)
#define PN532_RESET (22)
// #include <PN532/PN532_I2C/PN532_I2C.h>
// #include <PN532/PN532/PN532.h>
// #include <NfcAdapter.h>
// #include <PN532/PN532_SWHSU/PN532_SWHSU.h>
// #include <PN532/PN532_HSU/PN532_HSU.h>
//#include <SoftwareSerial.h>

const int RX1_PIN = GPIO_NUM_22;
const int TX1_PIN =  GPIO_NUM_19;

uint64_t NFC_READ_TIME = 0;


HardwareSerial HWSerial(Serial1);

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);

// PN532_HSU pn532hsu(Serial1);
// NfcAdapter nfc(pn532hsu);


// PN532_I2C pn532_i2c(Wire);
// NfcAdapter nfc = NfcAdapter(pn532_i2c);


// SoftwareSerial SWSerial(RX1_PIN, TX1_PIN);
// PN532_SWHSU pn532swhsu(SWSerial);
// NfcAdapter nfc = NfcAdapter(pn532swhsu);

long count = 0;
MyNeopixel* myNeopixel = new MyNeopixel();

void checkI2C(int _delay);

void setup() {
  M5.begin(true, false, false);

  // // Serial.begin(115200);
  // HWSerial.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);  

  // if (!HWSerial) { // If the object did not initialize, then its configuration is invalid
  //   Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
  //   M5.update();
  //   while (1) { // Don't continue with invalid configuration
  //     delay (1000);
  //   }
  // }
  // else
  // {
  //   Serial.printf("HWSerial Done\r\n");
  // }

  // Wire.setPins(25, 21);
  Wire.begin(25, 21);

  checkI2C(2000);

  nfc.begin();
  myNeopixel->InitNeopixel();
  
  Serial.println("Start Atom");
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 255), 50, 50);


  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  Serial.println("Waiting for an ISO14443A Card ...");

  delay(1000);
}

void loop() {
  if(M5.Btn.wasPressed())
  {
    count++;
    Serial.printf("Count : %d\r\n", count);
    // HWSerial.printf("Count : %d\r\n", count);
    
    // if (nfc.tagPresent()) {
    //     NdefMessage message = NdefMessage();
    //     message.addTextRecord("Hello, Arduino!");
    //     message.addUriRecord("http://arduino.cc");
    //     message.addTextRecord("Goodbye, Arduino!");
    //     boolean success = nfc.write(message);
    //     if (success) {
    //         Serial.println("Success. Try reading this tag with your phone.");
    //     } else {
    //         Serial.println("Write failed");
    //     }
    // }
    delay(50);
  }
  
  // M5.Btn.read();
  if (millis() - NFC_READ_TIME > 2000)
  {
    NFC_READ_TIME = millis();
    // Serial.println("\nScan a NFC tag\n");
    // if (nfc.tagPresent()) {
    //     NfcTag tag = nfc.read();
    //     tag.print();
    // }
  }
  
  M5.update();

  // checkI2C(2000);
}


void checkI2C(int _delay){
  delay(1000);
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(_delay); 
}
