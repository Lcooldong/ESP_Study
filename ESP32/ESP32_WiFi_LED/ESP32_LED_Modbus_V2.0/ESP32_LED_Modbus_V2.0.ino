#include "OTA.h"
#define EEPROM_SIZE 128
#include <ModbusIP_ESP8266.h>



int cnt = 0;

const int REG = 200;               // Modbus Hreg Offset
IPAddress remote(192, 168, 0, 236);  // Address of Modbus Slave device
const int LOOP_COUNT = 1;

ModbusIP mb;  //ModbusIP object

uint16_t res = 0;
uint8_t show = LOOP_COUNT;


void setup() {
  Serial.begin(115200);
  init_Neopixel(50);
  initOLED();
  EEPROM.begin(EEPROM_SIZE);
  setupOTA("ESP_LED");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  mb.client();
}

void loop() {
  reconnectWiFi();
  ArduinoOTA.handle();
  //pickOneLED(0, strip.Color(255,   0,   0), 50);
  rainbow(10);
  TelnetStream.println(cnt);
  cnt++;

  if (mb.isConnected(remote)) {   // Check if connection to Modbus Slave is established
    mb.readHreg(remote, REG, &res);  // Initiate Read Coil from Modbus Slave
  } else {
    mb.connect(remote);           // Try to connect if no connection
  }
  mb.task();                      // Common local Modbus task
  delay(1);                     // Pulling interval
  Serial.println(res);
  if (!show--) {                   // Display Slave register value one time per second (with default settings)
    //Serial.println(res);
    show = LOOP_COUNT;
  }

}
