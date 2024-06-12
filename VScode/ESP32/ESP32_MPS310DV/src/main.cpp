#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>


#define MPS310DV_ADDR 0x29



void checkI2C(int _delay){
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(_delay); 
}



void SensorRead(unsigned char addr, unsigned char* datbuf, unsigned char cnt) {
  Wire.beginTransmission(MPS310DV_ADDR);
  Wire.write(byte(addr));
  Wire.endTransmission();
  Wire.requestFrom(MPS310DV_ADDR, (int)cnt);
  if (cnt <= Wire.available()) {
    *datbuf++ = Wire.read();
    *datbuf = Wire.read();
  }
}


int ReadDistance(int _delay) {
  unsigned short distance;
  unsigned char data_buf[2];
  
  // 모듈의 0x00번지부터 2바이트 읽어오기
  // SensorRead(0x00, data_buf, 2);

  SensorRead(0x00, data_buf, 2);


 
  
  // Serial.printf("%d  | %d |\r\n", data_buf[0], data_buf[1]);
  // // 읽어온 두 바이트를 결합
  distance = data_buf[0] << 8;
  distance |= data_buf[1];

  delay(_delay);
  return distance;
}



void setup() {
  Serial.begin(115200);
  Wire.begin(); 

  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
  Serial.println("START");
}

void loop() {

  // checkI2C(3000);
  // int x = ReadDistance(100);
  // Serial.println(x);

    
}

