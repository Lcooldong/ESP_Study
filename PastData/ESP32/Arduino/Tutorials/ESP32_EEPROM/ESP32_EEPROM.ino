#include "EEPROM.h" //라이브러리 로드

void setup() {
  Serial.begin(115200);
  Serial.println("start");

  EEPROM.begin(100); //EEPROM의 메모리 주소 중 100까지 사용
  String ssid = "HELLO";
  String pwd = "WORLD";
  EEPROM.writeString(0, ssid); //EEPROM 주소 0 부터 ssid 기록
  EEPROM.writeString(20, pwd); //EEPROM 주소 20 부터 pwd 기록
  EEPROM.commit(); //기록한 내용을 저장
}

void loop() {
  Serial.println(EEPROM.readString(0));
  Serial.println(EEPROM.readString(20));
  delay(3000);
}
