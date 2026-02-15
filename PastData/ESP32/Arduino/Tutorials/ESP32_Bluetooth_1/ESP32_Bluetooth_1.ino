// ESP32 auto bluetooth

#include "BluetoothSerial.h"

// 블루투스 관련 정의 안되어있으면
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  // 시리얼 쓴 것 -> 블루투스로 전송
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  // 블루투스 받아온 것 -> 시리얼에 쓰기
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);
}
