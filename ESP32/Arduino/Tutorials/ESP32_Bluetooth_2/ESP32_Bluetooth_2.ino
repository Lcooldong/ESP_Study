// ESP32 auto bluetooth

#include "BluetoothSerial.h"
#define LED_PIN 15

// 블루투스 관련 정의 안되어있으면
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

String message = "";
char incomingChar;
int batteryValue = 0;
int batteryPin = 34;

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP_bl"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // 시리얼 쓴 것 -> 블루투스로 전송
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  
  // 블루투스 받아온 것 -> 시리얼에 쓰기
  // Read received messages (LED control command)
  if (SerialBT.available()){
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){    // 개행 문자로 안끝나면 계속 추가
      message += String(incomingChar);
    }
    else{
      message = ""; // 개행문자면 끝
    }
    Serial.write(incomingChar);  // 받은 것 계속 쓰기
  }
  // Check received message and control output accordingly
  if (message =="led_on"){
    digitalWrite(LED_PIN, HIGH);
    //Serial.println("Turn ON");
  }
  else if (message =="led_off"){
    digitalWrite(LED_PIN, LOW);
    //Serial.println("Turn OFF");
  }
  delay(20);
}

void readVolt(){
    batteryValue = analogRead(batteryPin);
    int volt = map(batteryValue,0,4095,0,4200);
    float result = (float)volt / 100;
    Serial.print("전압 : ");
    Serial.print(result);
    Serial.println("V");
}
