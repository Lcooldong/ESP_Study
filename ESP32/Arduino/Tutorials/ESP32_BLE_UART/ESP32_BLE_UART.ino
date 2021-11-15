#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


// Bluetooth Classic Serial Port Profile 같은 UART 구현 UUID
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"

// 편하게 쓰려고 포인터 미리 선언
BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic;
// 전송할 데이터
uint8_t txValue = 0;
bool deviceConnected = false;
bool oldDeviceConnected = false;


//ESP32 연결 상태 콜백함수
class MyServerCallbacks: public BLEServerCallbacks{
  // 클래스 맴버 함수 onConnect, onDisconnect
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
    // 받은 데이터를 rxValue변수에 저장
    std::string rxValue = pCharacteristic->getValue();
    //데이터가 있다면 true
    if(rxValue.length() > 0){
      Serial.println("******");
      Serial.print("Received Value: ");
      // string 출력
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
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // 송신 특성, UUID and NOTIFY
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  // Client가 데이터를 받아 속성을 읽거나 설정할 수 있기 위해 필요 
  pTxCharacteristic->addDescriptor(new BLE2902());  // UUID 2902 등록

  // 수신 특성
  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_RX,
                                          BLECharacteristic::PROPERTY_WRITE
                                         );
  // 수신 시 Callback 함수 호출                                       
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  
  // 서비스 시작
  pService->start();

  // advertising 시작, scan 시 표기됨
  pServer->getAdvertising()->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  char txBuffer[10];
  // 연결상태
  if (deviceConnected){
      sprintf(txBuffer, "%d\n", txValue);
      // 배열은 포인터(주소)
      pTxCharacteristic->setValue((uint8_t*)txBuffer, strlen(txBuffer));
      // notify 함수, 외부로 setValue 전송
      pTxCharacteristic->notify();
      txValue++;
      delay(1000);
  }

  // 연결 상태가 변경되었고 연결 해제된 상태
  if (!deviceConnected && oldDeviceConnected){
    delay(500);
    //advertising 시작
    pServer->startAdvertising();
    Serial.println("start advertising");
    // 이전 상태에 연결 상태 갱신
    oldDeviceConnected = deviceConnected;
  }

  // 연결 상태가 변경되었고 연결 된 상태
  if (deviceConnected && !oldDeviceConnected){
    // 이전 상태에 연결 상태 갱신  
    oldDeviceConnected = deviceConnected;
  }
}
