#include <Arduino.h>

#include <HardwareSerial.h>
#include <SoftwareSerial.h>


#define RX2_PIN 16
#define TX2_PIN 17

HardwareSerial mySerial(Serial2);

EspSoftwareSerial::UART myPort;
uint64_t lastTime = 0;

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
  // myPort.begin(38400, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  // if (!myPort) 
  // { // If the object did not initialize, then its configuration is invalid
  //   Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
  //   while (1) { // Don't continue with invalid configuration
  //     delay (1000);
  //   }
  // } 
}

bool ledToggleFlag = false;

void loop() {

  // Send
  if(millis() - lastTime > 1000)
  {
    lastTime = millis();
    if(ledToggleFlag)
    {
      mySerial.write('a');
      Serial.println("a");
    }
    else
    {
      mySerial.write('b');
      Serial.println("b");
    }

    ledToggleFlag = ! ledToggleFlag;
  }

  


  // Receive
  if(mySerial.available())
  {
    char receivedChar = mySerial.read();
    if(receivedChar == '1')
    {
      Serial.println("Get 1");
    }
    else if (receivedChar == '2')
    {
      Serial.println("Get 2");
    }
  }
}

