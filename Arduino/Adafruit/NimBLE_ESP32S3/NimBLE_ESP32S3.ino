#include <NimBLEDevice.h> // 공식 최신 버전 (v2.0.0+) 규격 반영

#define DEBUG_SERIAL Serial


const int MATRIX_WIDTH = 8;   // ROW
const int MATRIX_HEIGHT = 8;  // COLUMN

// BLE 고유 UUID 정의
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// 8x8 밝기 데이터를 저장할 로컬 버퍼 (초기값은 전부 꺼짐)
uint8_t ledBuffer[MATRIX_WIDTH * MATRIX_HEIGHT] = {0,};

// NimBLE 전역 객체 선언
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCharacteristic = nullptr;


// [공식 예제 반영 1] Server 콜백 구조체 최신 문법 교정
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        DEBUG_SERIAL.println("PC/스마트폰이 연결되었습니다! (NimBLE 최신 규격)");
        // 연결된 기기의 RSSI(신호세기) 등을 모니터링할 수도 있습니다.
    };
    
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        DEBUG_SERIAL.println("연결 해제됨. 다시 광고 시작...");
        // 공식 예제 방식: 디바이스 전역 함수를 통해 광고 재시작
        NimBLEDevice::startAdvertising(); 
    }
};

// [공식 예제 반영 2] Characteristic 콜백 구조체 최신 문법 교정
class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        // 최신 버전에서 데이터를 안전하게 읽어오는 방식
        std::string value = pChar->getValue();
        
        if (value.length() == 64) {
            // 수신된 64바이트를 전역 ledBuffer에 즉시 복사
            memcpy(ledBuffer, value.data(), 64);
        } else {
            DEBUG_SERIAL.print("데이터 길이 불일치: ");
            DEBUG_SERIAL.print(value.length());
            DEBUG_SERIAL.println(" 바이트 수신됨. (64바이트여야 합니다)");
        }
    }
};

void setup() {
  DEBUG_SERIAL.begin(115200);

  // 2. NimBLE 초기화 및 설정 (공식 예제 흐름 100% 매칭)
  NimBLEDevice::init("ESP32_NimBLE_MATRIX");
  
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  
  // 고속 스트리밍을 위한 Write Without Response(WRITE_NR) 설정
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::WRITE | 
                      NIMBLE_PROPERTY::WRITE_NR
                    );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  
  // 서비스 시작
  pService->start();

  // [공식 예제 반영 3] 최신 광고(Advertising) 패턴 적용
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  
  // 레거시 setScanResponse(true) 대신 공식 예제처럼 스캔 응답 데이터 구조 자동 활성화 처리
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();

  DEBUG_SERIAL.println("NimBLE 최신 서버 활성화 완료! 검색 가능합니다.");
}

void loop() {
  
}