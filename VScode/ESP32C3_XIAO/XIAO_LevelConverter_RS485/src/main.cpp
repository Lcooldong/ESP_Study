#include <Arduino.h>
// #include <SoftwareSerial.h>
#include <HardwareSerial.h>

const int RX_PIN = D0;
const int TX_PIN = D1;

const int analogTest_PIN = D2;
const int levelConverter_PIN = D3;

const int lowLevel_PIN = D10;

uint64_t levelConverterTime = 0;
bool levelConverterState = false;

const int ledChannel_0 = 0;
uint64_t ledcTime = 0;
bool ledDirection = 0;
uint8_t ledValue = 0;

uint64_t rs485Time = 0;
uint64_t count = 0;

HardwareSerial RS485(Serial1);

// EspSoftwareSerial::Config swSerialConfig = EspSoftwareSerial::SWSERIAL_8E1;
// EspSoftwareSerial::Parity parity = EspSoftwareSerial::PARITY_NONE;
// EspSoftwareSerial::UART RS485; 

void setup() {
  Serial.begin(115200);
  RS485.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN, false, 100);
  // RS485.begin(9600, swSerialConfig, RX_PIN, TX_PIN);

  pinMode(levelConverter_PIN, OUTPUT);
  pinMode(lowLevel_PIN, INPUT);

  ledcAttachPin(analogTest_PIN, ledChannel_0);
  ledcSetup(ledChannel_0, 5000, 8);
  

}

void loop() {
  if(millis() - ledcTime > 10)
  {
    if(ledValue == 0)
    {
      ledDirection = true;
    }
    else if(ledValue == 255)
    {
      ledDirection = false;
    }

    if(ledDirection)
    {
      ledValue++;
    }
    else
    {
      ledValue--;
    }

    ledcWrite(ledChannel_0, ledValue);

    ledcTime = millis();
  }

  if(millis() - levelConverterTime > 100)
  {
    digitalWrite(levelConverter_PIN, levelConverterState);

    levelConverterState = !levelConverterState;

    levelConverterTime = millis();
  }

  if(millis() - rs485Time > 1000)
  { 
    count++;

    RS485.println(count);
    Serial.printf("[%d]\r\n", count);
    rs485Time = millis();
  }

}

