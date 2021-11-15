#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include "DHT.h"

/*
- ADC1 : ADC1_0(36번핀), ADC1_3(39번핀), ADC1_4(32번핀),  

         ADC1_5(33번핀), ADC1_6(34번핀), ADC1_7(35번핀)

- ADC2 : ADC2_0(4번핀), ADC2_2(2번핀), ADC2_3(15번핀), 

         ADC2_4(13번핀),  ADC2_5(12번핀), ADC2_6(14번핀), 

         ADC2_7(27번핀),ADC2_8(25번핀),ADC2_9(26번핀)
*/

#define DHTPIN 15
#define DHTTYPE DHT11
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"

DHT dht(DHTPIN, DHTTYPE);
BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic;

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

bool deviceConnected = false;
bool oldDeviceConnected = false;

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
  BLEDevice::init("ESP32 DHT11");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      ); 
  pTxCharacteristic->addDescriptor(new BLE2902());      
               
  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_RX,
                                          BLECharacteristic::PROPERTY_WRITE
                                         );                                 
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  
  
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  dht.begin();
}

void loop() {
  char txBuffer[20];
  float h = dht.readHumidity();// 습도를 측정합니다.
  float t = dht.readTemperature();// 온도를 측정합니다.
  float f = dht.readTemperature(true);// 화씨 온도를 측정합니다.

  // 값 읽기에 오류가 있으면 오류를 출력합니다.
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // 보정된 화씨 값을 가져옵니다.
  float hif = dht.computeHeatIndex(f, h);
  // 보정된 섭씨 값을 가져옵니다.
  float hic = dht.computeHeatIndex(t, h, false);

  
  // 연결상태
  if (deviceConnected){
      sprintf(txBuffer, "%d %d\n", h, t);
      Serial.println(h);
      Serial.println(t);
      // 배열은 포인터(주소)
      pTxCharacteristic->setValue((uint8_t*)txBuffer, strlen(txBuffer));
      // notify 함수, 외부로 setValue 전송
      pTxCharacteristic->notify();
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







//  Serial.print("Humidity: ");
//  Serial.print(h);
//  Serial.print(" | ");
//  Serial.print("Temperature: ");
//  Serial.print(t);
//  Serial.print(" *C |");
//  Serial.print(f);
//  Serial.print(" *F |");
//  Serial.print("Heat index: "); //열지수, 체감온도
//  Serial.print(hic);
//  Serial.print(" *C |");
//  Serial.print(hif);
//  Serial.println(" *F");
}
