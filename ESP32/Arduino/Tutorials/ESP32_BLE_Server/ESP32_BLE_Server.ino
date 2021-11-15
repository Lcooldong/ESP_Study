// https://m.blog.naver.com/chandong83/222024830051 자료 

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h> // BLE 관련 헤더 파일들

// 서비스 UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// 서비스 UUID 내 사용되는 캐릭터리스틱 UUID
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  //디버깅용 시리얼 
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  //BLEDevice 클래스 초기화 "Long name works now" 라는 이름의 장치
  BLEDevice::init("Long name works now");
  // 서버(Peripheral) 생성
  BLEServer *pServer = BLEDevice::createServer();
  // 서비스 생성
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // 서비스 내에 캐릭터리스틱 1개 추가
  // 해당 캐릭터리스틱은 읽기 / 쓰기만 지원함.
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  // 캐릭터리스틱에 "Hello World says Neil"이라는 내용 쓰기
  pCharacteristic->setValue("Hello World says Neil");
  // 서비스 시작
  pService->start();

  // 어드버타이징(브로드케스팅 정도로 이해하면 됨) 관련
  // 어드버타이징 구조체 얻기 
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  // 어드버타이징에 서비스 UUID를 등록
  pAdvertising->addServiceUUID(SERVICE_UUID);

  // 어드버타이징에 긴 데이터를 보낼수 있게 함(31바이트 이상) 스켄리스폰스 활성
  pAdvertising->setScanResponse(true);

  // central <-> peripheral 사이의 연결 파라미터, 블루투스 표준 1.25ms 간격
  // 커넥션 인터벌용 설정 0x06이면 6 * 1.25ms = 7.5ms
  // setMinPreferred(0x06)이면 최소 7.5ms까지 통신(연결) 간격을 허용한다는 설정
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue

  // 좀 이해는 안되는 부분인데... 
  // pAdvertising->setMaxPreferred(0x12); 가 맞을듯 싶다.
  // setMaxPreferred 라고 가정하고 0x12 면 18(10진수) * 1.25ms = 22.5ms
  // 최도 22.5ms까지 통신 간격을 허용한다는 설정 
  //pAdvertising->setMinPreferred(0x12);
  pAdvertising->setMaxPreferred(0x12);

  // 어드버타이징 시작   
  BLEDevice::startAdvertising();

  //디버그 메시지 출력
  //장치 연결해서 읽어보라
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  delay(2000);
}
