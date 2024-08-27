#include <Arduino.h>
#include <HardwareSerial.h>
// #include <SoftwareSerial.h>
#include <Dynamixel2Arduino.h>
#include "config.h"
// #include <DynamixelSDK.h>



const int RX_PIN = D0;
const int TX_PIN = D1;

const int analogTest_PIN = D2;
const int DIR_PIN = D3;


uint64_t levelConverterTime = 0;
bool levelConverterState = false;

const int ledChannel_0 = 0;
uint64_t ledcTime = 0;
bool ledDirection = 0;
uint8_t ledValue = 0;

bool DXL_ledState = false;
uint64_t rs485Time = 0;
uint64_t count = 0;

HardwareSerial RS485(Serial1);

// EspSoftwareSerial::UART RS485(RX_PIN, TX_PIN);
// EspSoftwareSerial::Config swSerialConfig = EspSoftwareSerial::SWSERIAL_8N1;

// Dynamixel2Arduino dxl(RS485, DIR_PIN);
Dynamixel2Arduino dxl(RS485);

using namespace ControlTableItem;


// dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
// dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

// int dxl_comm_result = COMM_TX_FAIL;             // Communication result

// uint8_t dxl_error = 0;                          // Dynamixel error
// uint16_t dxl_model_number; 

void setup() {
  Serial.begin(115200);
  while(!Serial){};

  // RS485.begin(BAUDRATE, swSerialConfig);
  RS485.begin(BAUDRATE,  RX_PIN, TX_PIN);




  ledcAttachPin(analogTest_PIN, ledChannel_0);
  ledcSetup(ledChannel_0, 5000, 8);
  
  dxl.begin();
  dxl.setOperatingMode(DXL_ID, OP_POSITION);
  dxl.torqueOn(1);
  dxl.setPortProtocolVersion(PROTOCOL_VERSION);
  dxl.ping(DXL_ID);



  


  Serial.println("--Start Dynamixel--");
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

  if(millis() - rs485Time > 1000)
  { 
    count++;
    if(!DXL_ledState)
    {
      dxl.ledOn(DXL_ID);
      dxl.setGoalPosition(DXL_ID, 0);
    }
    else
    {
      dxl.ledOff(DXL_ID);
      dxl.setGoalPosition(DXL_ID, 512);
    }

    DXL_ledState = !DXL_ledState;
    // RS485.println(count);
    Serial.printf("[%d]\r\n", count);
    rs485Time = millis();
  }

}

