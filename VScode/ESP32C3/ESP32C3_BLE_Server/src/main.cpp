#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "neopixel.h"

#include <iostream>
using namespace std;
#define M5STAMP_C3
//#define ESP32C3_LCD

#ifdef M5STAMP_C3
  #define BTN_PIN 3
#endif  

#ifdef ESP32C3_LCD  // -> 버그있음, 또는 보드 문제
  #include <SPI.h>
  #include <Wire.h>
  #include <U8g2lib.h>
  #define SDA_PIN 5
  #define SCL_PIN 6
  #define VOLT_INPUT_PIN 3

  U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); 
#endif

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristicTX = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26TX"
#define CHARACTERISTIC_UUID_RX "beb5483e-36e1-4688-b7f5-ea07361b26RX"

volatile uint32_t cnt = 0;
// Interval between internal temperature reads
unsigned long next_temp_read = 0;   // Next time step in milliseconds
uint16_t temp_read_interval = 1000;  // This is in milliseconds

MyNeopixel* myNeopixel = new MyNeopixel();


// 서버 콜백함수 - 연결 상태 확인용
class MyServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(0, 0, 255), 50, 50);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 50);
    }
};

// 캐릭터 콜백 - 값 받아오기용
class MyCallbacks: public BLECharacteristicCallbacks 
{

    void onWrite(BLECharacteristic *pCharacteristicTX) {
      std::string value = pCharacteristicTX->getValue();
      
      //String receivedText = pCharacteristicTX->getValue().c_str();
      String receivedText = value.c_str();
      //const char* receivedCharText = value.c_str();
      //String receivedText = (String)receivedCharText;


      // Get raw data
      if (value.length() > 0) 
      {
        Serial.print("Received value Array : ");
        for (int i = 0; i < value.length(); i++)
        {
          Serial.print(value[i]);
        }
        Serial.println();
        //Serial.printf("Received Value : %s\r\n", receivedCharText);
        //Serial.printf("Received Value : %s\r\n", receivedText);

        int redIndex = receivedText.indexOf(",");
        int greenIndex = receivedText.indexOf(",", redIndex + 1);
        int blueIndex = receivedText.indexOf(",", greenIndex + 1);
        int length = receivedText.length();

        String red = receivedText.substring(0, redIndex);
        String green = receivedText.substring(redIndex + 1, greenIndex);
        String blue = receivedText.substring(greenIndex + 1, blueIndex);
        String brightness = receivedText.substring(blueIndex + 1, length);

        myNeopixel->pickOneLED(0, myNeopixel->strip->Color(red.toInt(), green.toInt(), blue.toInt()), brightness.toInt(), 10);

      }
    }

};



void setup() {
  Serial.begin(115200);
  myNeopixel->InitNeopixel();
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(255, 0, 0), 50, 1);
#ifdef M5STAMP_C3
  pinMode(BTN_PIN, INPUT_PULLUP);
#endif

#ifdef ESP32C3_LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
  u8g2.sendBuffer();

#endif


  BLEDevice::init("ESP32C3_BLE_TEST");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());                 // 콜백함수
  BLEService *pService = pServer->createService(SERVICE_UUID);    // 서비스 생성


  // 캐릭터 설정 - 보드 송신 부분
  pCharacteristicTX = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,                     // TX UUID - 보통 NOTIFY 사용
//                      BLECharacteristic::PROPERTY_READ   |
//                      BLECharacteristic::PROPERTY_WRITE  |
//                      BLECharacteristic::PROPERTY_NOTIFY |
//                      BLECharacteristic::PROPERTY_INDICATE
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // 디스크립터 - 보드 수신 부분
  pCharacteristicTX->addDescriptor(new BLE2902());
  BLECharacteristic * pCharacteristicRX = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,                    // RX UUID - WRITE 사용
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristicRX->setCallbacks(new MyCallbacks());             // 수신 데이터 콜백함수
  pService->start();

  // Advertising 하는 부분
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);  // When PC Connecting to Device
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");


}

uint64_t lastTime = 0;
uint16_t interval = 1000;

void loop() {

  if(millis() - lastTime > interval){
    lastTime = millis();
    Serial.printf("COUNT : %d\r\n", cnt++);

  }
#ifdef M5STAMP_C3
  else if(digitalRead(BTN_PIN) == LOW)
  {
    Serial.println("Button Pressed"); 
    while(true) // 버튼 누를 때 완전 멈춤
    {
      if(digitalRead(BTN_PIN) == HIGH)
      {
        break;
      }
    };
    delay(10);
  }
#endif

#ifdef ESP32C3_LCD
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10);
  u8g2.printf("BLE Light");
  u8g2.setCursor(0, 20);
  u8g2.printf("Battery : %d", cnt);
  
  //int voltData = analogRead(VOLT_INPUT_PIN);
  //const char* buffer = voltData.c_str();
  u8g2.sendBuffer();
  delay(1000);
#endif
  myNeopixel->pickOneLED(0, myNeopixel->strip->Color(50, 0, 200), 50, 1);



  // 연결된 상태
  if (deviceConnected) {
      //pCharacteristicTX->setValue((uint8_t*)&value, 4);
      char buf[20]="";
      sprintf(buf,"%5d",value);
      Serial.print(" TX Value : ");
      Serial.println(buf);
      value++;
      
      pCharacteristicTX->setValue(buf);
      pCharacteristicTX->notify();
      delay(3000);
      //delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }



  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }


}

