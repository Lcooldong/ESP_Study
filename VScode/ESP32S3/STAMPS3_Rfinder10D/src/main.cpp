#include <Arduino.h>
#include <Mecha_Rfinder10D.h>

//#define MULTI_TARGET
#define SINGLE_TARGET

Mecha_Rfinder10D rfinder;
HardwareSerial mySerial(1);


void setup() {
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 1, 3);//RX,TX

  if (!rfinder.init(mySerial)) {
    Serial.println("Init error!");
    while (true) {
      delay(1000);
    }
  }
}

void loop() {
    rfinder.fetchNewData(); // Serial1의 데이터를 polling함으로, 항상 실행되어야 함.

  if (rfinder.isNewData()) {                  // 마지막으로 .isNewData() 메소드 실행 후 새로운 데이터가 있다면,


#ifdef MULTI_TARGET
  int rfinder_cnt = rfinder.getTargetCnt(); // 인식된 타겟 수 확인
    
  Serial.print("Target[");
  Serial.print(rfinder_cnt);
  Serial.print("]=>");

    for (int i = 0; i < rfinder_cnt; i++) { // 타겟 수 만큼 출력
      Serial.print("Range : ");
      Serial.print(rfinder.getRange(i)); // .getRange() 메소드의 인자값으로 타겟 번호 전달
      Serial.print("\t\t");
    }
    Serial.println();
  
#else
  Serial.print(">Strength : ");
  Serial.println(rfinder.getStrength()); // 인자값으로 아무것도 전달하지 않는 경우 가장 근처의 타겟으로 정해짐
  // Serial.print("\t");
  Serial.print(">Range : ");
  Serial.println(rfinder.getRange());

#endif
  }
}

