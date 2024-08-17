#include <Arduino.h>
#include <SoftwareSerial.h>

const int RS485_RX = GPIO_NUM_10;
const int RS485_TX = GPIO_NUM_9;

EspSoftwareSerial::UART RS485;


void setup() {
  Serial.begin(115200);
  RS485.begin(57600, EspSoftwareSerial::SWSERIAL_8N1, RS485_RX, RS485_TX);
}

void loop() {
  RS485.write("1");
  delay(1000);
}

