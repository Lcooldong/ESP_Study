#include <NimBLEDevice.h>

#define DEBUG_SERIAL Serial

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

// 서버 연결 상태 콜백
class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
      DEBUG_SERIAL.println(">> PC 클라이언트 연결됨!");
      deviceConnected = true;
  }
  
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
      DEBUG_SERIAL.println(">> 연결 해제됨. 다시 광고(Advertising) 시작...");
      deviceConnected = false;
      NimBLEDevice::startAdvertising(); 
  }
};

// 데이터 수신 콜백
class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        std::string value = pChar->getValue();
        
        if (value.length() > 0) {
            char command = value[0];
            DEBUG_SERIAL.print(">> 수신된 명령어: ");
            DEBUG_SERIAL.println(command);

            if (command == '1') {
                digitalWrite(LED_BUILTIN, LOW);  // 내장 LED 켜기
                DEBUG_SERIAL.println("LED ON");
                
                // [변경] 현재의 상태('1')를 우체통에 세팅하고, 연결된 PC에 실시간 알림(Notify)을 보냄
                pChar->setValue("1");
                pChar->notify();
            } 
            else if (command == '0') {
                digitalWrite(LED_BUILTIN, HIGH);   // 내장 LED 끄기
                DEBUG_SERIAL.println("LED OFF");
                
                // [변경] 현재의 상태('0')를 우체통에 세팅하고, 연결된 PC에 실시간 알림(Notify)을 보냄
                pChar->setValue("0");
                pChar->notify();
            }
        }
    }
};

void setup() {
  DEBUG_SERIAL.begin(115200);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // HIGH is OFF

  NimBLEDevice::init("ESP32_BLE_TestPin");
  DEBUG_SERIAL.print("★ 이 보드의 BLE MAC 주소: ");
  DEBUG_SERIAL.println(NimBLEDevice::getAddress().toString().c_str());

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  
  // [수정] WRITE 뿐만 아니라 READ와 NOTIFY 속성을 함께 부여합니다.
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::WRITE |
                      NIMBLE_PROPERTY::READ  |
                      NIMBLE_PROPERTY::NOTIFY
                    );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  
  // 초기 상태 설정 (처음엔 꺼져 있으므로 '0')
  pCharacteristic->setValue("0");
  
  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->enableScanResponse(true);
  // pAdvertising->setScanResponseData(true)
  // pAdvertising->start();
  
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); 
  NimBLEDevice::startAdvertising();

  DEBUG_SERIAL.println("아두이노 준비 완료! 상태 피드백 가능.");
}

void loop() {
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(1000);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(1000);
  
}