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
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 50);


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
  
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    uint8_t cardList[255];
    uint8_t purpleCard[4] = {0x2F, 0x70, 0x94, 0x05};
    uint8_t greenCard[4] = {0xE4, 0xB0, 0x5E, 0x2A};
    bool purpleSelect = false;
    bool greenSelect = false;

    for (int i = 0; i < uidLength; i++)
    {
      if(uid[i] == purpleCard[i])
      {
        purpleSelect = true;
      }
      else
      {
        purpleSelect = false;
      }

      if(uid[i] == greenCard[i])
      {
        greenSelect = true;
      }
      else
      {
        greenSelect = false;
      }


    }
    
    if (purpleSelect)
    {
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 255), 50, 1);
    }
    else if(greenSelect)
    {
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 255, 0), 50, 1);
    }
    else
    {
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 1);
    }
    


    
    // if (uidLength == 7)
    // {
    //   uint8_t data[32];

    //   // We probably have an NTAG2xx card (though it could be Ultralight as well)
    //   Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");

    //   // NTAG2x3 cards have 39*4 bytes of user pages (156 user bytes),
    //   // starting at page 4 ... larger cards just add pages to the end of
    //   // this range:

    //   // See: http://www.nxp.com/documents/short_data_sheet/NTAG203_SDS.pdf

    //   // TAG Type       PAGES   USER START    USER STOP
    //   // --------       -----   ----------    ---------
    //   // NTAG 203       42      4             39
    //   // NTAG 213       45      4             39
    //   // NTAG 215       135     4             129
    //   // NTAG 216       231     4             225

    //   for (uint8_t i = 0; i < 42; i++)
    //   {
    //     success = nfc.ntag2xx_ReadPage(i, data);

    //     // Display the current page number
    //     Serial.print("PAGE ");
    //     if (i < 10)
    //     {
    //       Serial.print("0");
    //       Serial.print(i);
    //     }
    //     else
    //     {
    //       Serial.print(i);
    //     }
    //     Serial.print(": ");

    //     // Display the results, depending on 'success'
    //     if (success)
    //     {
    //       // Dump the page data
    //       nfc.PrintHexChar(data, 4);
    //     }
    //     else
    //     {
    //       Serial.println("Unable to read the requested page!");
    //     }
    //   }
    // }
    // else
    // {
    //   Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    // }

    // // Wait a bit before trying again
    // Serial.println("\n\nSend a character to scan another tag!");
    // Serial.flush();
    // while (!Serial.available());
    // while (Serial.available()) {
    // Serial.read();
    // }
    // Serial.flush();
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
