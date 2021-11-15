#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


// Bluetooth Serial Port Profile 같은 UART 구현 UUID
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"

// 편하게 쓰려고 포인터 미리 선언
BLEServer* pServer = NULL;
BLECharacteristic * pTxCharacteristic;
uint8_t txValue = 0;
bool deviceConnected = false;
bool oldDeviceConnected = false;


//ESP32 연결 상태 콜백함수
class MyServerCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer* pServer){
    deviceConnected = true;
  }  
  void onDisconnect(BLEServer* pServer){
    deviceConnected = false;
  }
};

// ESP32 데이터를 입력받는 콜백함수
class MyCallbacks: public BLECharacteristicCallbacks{
  // 데이터 받아옴
  void onWrite(BLECharacteristic* pCharacteristic){
    std::string rxValue = pCharacteristic->getValue();

    if(rxValue.length() > 0){
      Serial.println("******");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++)
        Serial.print(rxValue[i]);
      Serial.println();
      Serial.println("******");  
    }
  }  
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  // BLE 생성
  BLEDevice::init("ESP32_BLE_SERVER");
  // 서버 생성
  pServer = BLEDevice::createServer();
  // 연결 상태 콜백함수 등록
  pServer->setCallbacks(new MyServerCallbacks());

  // 서비스 UUID 등록
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 송신 특성
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  pTxCharacteristic->addDescriptor(new BLE2902());

  // 수신 특성
  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_RX,
                                          BLECharacteristic::PROPERTY_WRITE
                                         );
  // 수신 시 Callback 함수 호출                                       
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  
  // 서비스 시작
  pService->start();

  // advertising 시작
  pServer->getAdvertising()->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  char txBuffer[10];
  if (deviceConnected){
      sprintf(txBuffer, "%d\n", txValue);
      pTxCharacteristic->setValue((uint8_t*)txBuffer, strlen(txBuffer));
      pTxCharacteristic->notify();
      txValue++;
      delay(1000);
  }

  if (!deviceConnected && oldDeviceConnected){
    delay(500);

    pServer->startAdvertising();
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected){
      
    oldDeviceConnected = deviceConnected;
  }
}
