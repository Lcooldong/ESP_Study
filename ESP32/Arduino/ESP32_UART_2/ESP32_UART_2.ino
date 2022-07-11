#include <SoftwareSerial.h>

#define MYPORT_TX 17
#define MYPORT_RX 16

SoftwareSerial myPort;

void setup() {
  Serial.begin(115200); // Standard hardware serial port

  myPort.begin(115200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (!myPort) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid SoftwareSerial pin configuration, check config"); 
    while (1) { // Don't continue with invalid configuration
    delay (1000);
    }
  } 

}

void loop() {
  while (myPort.available() > 0) {
    Serial.write(myPort.read());
  }
  while (Serial.available() > 0) {
    myPort.write(Serial.read());
  }

}
