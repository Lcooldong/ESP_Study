#include <Arduino.h>
#include "main.h"
#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define BTN 3
#define LED_COUNT 60
#define LED_PIN    2

#define SERVICE_UUID        "DEAD"
#define CHARACTERISTIC_UUID "BEEF"


bool BTN_flag = false;

static NimBLEServer* pServer;

unsigned long current_time = 0;
unsigned long last_time = 0;
uint16_t interval = 1000; 

int count = 0;


unsigned long pixelPrevious = 0;        // Previous Pixel Millis
unsigned long patternPrevious = 0;      // Previous Pattern Millis
int           patternCurrent = 0;       // Current Pattern Number
int           patternInterval = 5000;   // Pattern Interval (ms)
int           pixelInterval = 50;       // Pixel Interval (ms)
int           pixelQueue = 0;           // Pattern Pixel Queue
int           pixelCycle = 0;           // Pattern Pixel Cycle
uint16_t      pixelCurrent = 0;         // Pattern Current Pixel Number
uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels

int RGB_Array[5] = {0,};

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);



/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
// 연결하는 부분
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected");
        Serial.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };
    /** Alternative onConnect() method to extract details of the connection.
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        Serial.print("Client address: ");
        Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 5x interval time for best results.
         */
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };

/********************* Security handled here **********************
****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
        Serial.println("Server Passkey Request");
        /** This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key){
        Serial.print("The passkey YES/NO number: ");Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc){
        /** Check that encryption was successful, if not we disconnect the client */
        if(!desc->sec_state.encrypted) {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            Serial.println("Encrypt connection failed - disconnecting client");
            return;
        }
        Serial.println("Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
        String received_data = pCharacteristic->getValue().c_str();
        Serial.println(received_data);
        parcing_rgb(received_data);
        pickOneLED(0, strip.Color(RGB_Array[0], RGB_Array[1], RGB_Array[2]), RGB_Array[3], 20);
        switch (RGB_Array[4])
        {
        case 1:
          pickOneLED(0, strip.Color(RGB_Array[0], RGB_Array[1], RGB_Array[2]), RGB_Array[3], 20);
          break;
        case 2:
          pickOneLED(1, strip.Color(RGB_Array[0], RGB_Array[1], RGB_Array[2]), RGB_Array[3], 20);
          break;
        case 3:
          pickOneLED(2, strip.Color(RGB_Array[0], RGB_Array[1], RGB_Array[2]), RGB_Array[3], 20);
          break;
        case 4:
          pickOneLED(3, strip.Color(RGB_Array[0], RGB_Array[1], RGB_Array[2]), RGB_Array[3], 20);
          break;
        case 5:
          pickOneLED(4, strip.Color(RGB_Array[0], RGB_Array[1], RGB_Array[2]), RGB_Array[3], 20);
          break;
        default:

          break;
        }

    };
    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", ";
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
        std::string dscVal = pDescriptor->getValue();
        Serial.print("Descriptor witten value:");
        Serial.println(dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor) {
        Serial.print(pDescriptor->getUUID().toString().c_str());
        Serial.println(" Descriptor read");
    };
};


/** Define callback instances globally to use for multiple Charateristics \ Descriptors */
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;





void setup() {
  Serial.begin(115200);
  pinMode(BTN, INPUT_PULLUP);
  neopixel_init();
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32_BLE_Server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID,
                                  /***** Enum Type NIMBLE_PROPERTY now *****      
                                        BLECharacteristic::PROPERTY_READ   |
                                        BLECharacteristic::PROPERTY_WRITE  
                                        );
                                  *****************************************/
                                        NIMBLE_PROPERTY::READ |
                                        NIMBLE_PROPERTY::WRITE 
                                       );
  pCharacteristic->setCallbacks(&chrCallbacks);
  pCharacteristic->setValue("Hello World says Neil");
 
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);
  
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}



// ascii 
// 0x30 = 0
// 0x41 = A
// 0x61 = a


void loop() {
  /** Do your thing here, this just spams notifications to all connected clients */


  if(digitalRead(BTN) == false){
    delay(5);
    if(digitalRead(BTN) == false){
      NimBLEService* pSvc = pServer->getServiceByUUID("DEAD");
      NimBLECharacteristic* pChr = pSvc->getCharacteristic("BEEF");
      Serial.println(pChr->getValue().c_str());

    //   pChr->setValue(count);
    //   Serial.print("0x");
    //   Serial.println(count, HEX);
    //   count++;
      while(digitalRead(BTN) == false){
        ;;
      }

    }
  }
  delay(50);


}


void parcing_rgb(String RGB){
  int first_comma = RGB.indexOf(",");
  int second_comma = RGB.indexOf(",", first_comma + 1);
  int third_comma = RGB.indexOf(",", second_comma + 1);
  int forth_comma = RGB.indexOf(",", third_comma + 1);
  int RGB_length = RGB.length();
  
  

  int red = RGB.substring(0, first_comma).toInt();
  int green = RGB.substring(first_comma + 1, second_comma).toInt();
  int blue = RGB.substring(second_comma + 1, third_comma).toInt();
  int brightness = RGB.substring(third_comma + 1, forth_comma).toInt();
  int text_flag = RGB.substring(forth_comma + 1 ,RGB_length).toInt();

  RGB_Array[0] = red;
  RGB_Array[1] = green;
  RGB_Array[2] = blue;
  RGB_Array[3] = brightness;
  RGB_Array[4] = text_flag;
}


void neopixel_init(void){
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

}


void pickOneLED(uint8_t ledNum, uint32_t color, uint8_t brightness, int wait){
    strip.setBrightness(brightness);
    strip.setPixelColor(ledNum, color);  
    strip.show();                                               
    delay(wait);
}

void blinkNeopixel(uint32_t color, int times, int delays){
  for(int i = 0; i < times; i++){
    pickOneLED(0, color, 50, delays);
    pickOneLED(0, strip.Color(0, 0, 0), 0, delays);
  }
}

void resetNeopixel(void){
  for(int i=0; i < 256; i++){
    pickOneLED(i, strip.Color(0, 0, 0), 0, 0 );
  } 
}



void colorWipe(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  strip.setPixelColor(pixelCurrent, color); //  Set pixel's color (in RAM)
  strip.show();                             //  Update strip to match
  pixelCurrent++;                           //  Advance current pixel
  if(pixelCurrent >= pixelNumber)           //  Loop the pattern from the first LED
    pixelCurrent = 0;
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  for(int i = 0; i < pixelNumber; i++) {
    strip.setPixelColor(i + pixelQueue, color); //  Set pixel's color (in RAM)
  }
  strip.show();                             //  Update strip to match
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Set pixel's color (in RAM)
  }
  pixelQueue++;                             //  Advance current pixel
  if(pixelQueue >= 3)
    pixelQueue = 0;                         //  Loop the pattern from the first LED
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   
  for(uint16_t i=0; i < pixelNumber; i++) {
    strip.setPixelColor(i, Wheel((i + pixelCycle) & 255)); //  Update delay time  
  }
  strip.show();                             //  Update strip to match
  pixelCycle++;                             //  Advance current cycle
  if(pixelCycle >= 256)
    pixelCycle = 0;                         //  Loop the cycle back to the begining
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time  
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, Wheel((i + pixelCycle) % 255)); //  Update delay time  
  }
  strip.show();
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Update delay time  
  }      
  pixelQueue++;                           //  Advance current queue  
  pixelCycle++;                           //  Advance current cycle
  if(pixelQueue >= 3)
    pixelQueue = 0;                       //  Loop
  if(pixelCycle >= 256)
    pixelCycle = 0;                       //  Loop
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}