#include <Arduino.h>
#include <TinyPICO.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

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

// Initialise the TinyPICO library
TinyPICO tp = TinyPICO();

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristicTX) {
      std::string value = pCharacteristicTX->getValue();

      if (value.length() > 0) {
        Serial.print("Receved value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);
        Serial.println();
      }
    }
};



void setup() {
  Serial.begin(115200);

  tp.DotStar_SetPixelColor(0x0F000F);
  tp.DotStar_Show();

  BLEDevice::init("Gripper_Test");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristicTX = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristicTX->addDescriptor(new BLE2902());
  BLECharacteristic * pCharacteristicRX = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristicRX->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);  // When PC Connecting to Device
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");


}

uint32_t lastTime = 0;
uint16_t interval = 1000;

void loop() {

  if(millis() - lastTime > interval){
    lastTime = millis();
    Serial.printf("COUNT : %d\r\n", cnt++);
    //tp.DotStar_CycleColor(10);
  }

    
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

