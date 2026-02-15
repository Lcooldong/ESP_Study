#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define LED_PIN 15  // For Test
#define LED 22
#define batteryPin 34

String message = "";
String command = "";
char incomingChar;

int sensorValue = 0;
float calibration = 0.2;
float voltage;
int bat_percentage;

int blink_switch = 0;
int ledState = LOW;
unsigned long previousMillis = 0;
long interval = 1000; // 변경 가능해야함
unsigned long previousMillis2 = 0;
const long interval2 = 2000;

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("LED_Remote"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  ledcSetup(0, 5000, 8);
  ledcAttachPin(LED_PIN, 0);
  pinMode(LED, OUTPUT);
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
  int secondIndex1 = message.indexOf('!');
  int secondIndex2 = message.indexOf('s');  // s 전까지는 -1
  int size_of_message = message.length();   // 문자열 끝은 \0까지 있음

  command = message.substring(0, secondIndex1);
  int _time = message.substring(secondIndex1+1, secondIndex2).toInt();
//  delay(500);
  if ((_time != 0) && (secondIndex2 == size_of_message-1)){
    interval = _time;
  }

//  Serial.print("s 찾기 : ");
//  Serial.println(secondIndex2);
//  Serial.print("전체길이 : ");
//  Serial.println(size_of_message);
//  Serial.print("간격 : ");
//  Serial.println(interval);
  
  if (message =="led_on"){
    digitalWrite(LED, HIGH);
    //Serial.println("Turn ON");
  }
  else if (message =="led_off"){
    digitalWrite(LED, LOW);
    //Serial.println("Turn OFF");
  }else if (command == "blink_on"){
    blink_switch = 1;
  }else if (message == "blink_off"){
    digitalWrite(LED, LOW);
    blink_switch = 0;
  }

  if (blink_switch == 1){
    blink_led();
  }
  
  delay(20);
  batteryCheck();
  SerialBT.println(String(voltage) + "," + String(bat_percentage) + ",");
}



void led_control(){
  for(int i=0;i<=255; i++)
  {
    ledcWrite(0, i);
    Serial.println(i);
    delay(100);
  }
}

void blink_led(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    if (ledState == LOW){
      ledState = HIGH;  
    }else{
      ledState = LOW;  
    }
    digitalWrite(LED, ledState);
  }
}

void batteryCheck(){
  unsigned long currentMillis2 = millis();
  if (currentMillis2 - previousMillis2 >= interval2){
    previousMillis2 = currentMillis2;
    sensorValue = analogRead(batteryPin);
    //Serial.print("sensor : ");
    //Serial.println(sensorValue);
  
    voltage = (((sensorValue * 3.3) / 4095) * 2 + calibration);
  
    //Serial.print("VOLT : ");
    //Serial.print(voltage);
    //Serial.println("V"); 

    
    bat_percentage = (float)map((int)(voltage * 1000), 2500, 4200, 0, 100);
  
    if (bat_percentage >= 100){
      bat_percentage = 100;  
    }
    if (bat_percentage <= 0){
      bat_percentage = 1;
    }
    //Serial.print("percentage : ");
    //Serial.print(bat_percentage);
    //Serial.println("%");
  } 
}
