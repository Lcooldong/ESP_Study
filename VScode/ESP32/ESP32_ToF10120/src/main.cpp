#include <Arduino.h>
#include <Wire.h>


#define ToF250_ADDR 0x52 // ToF250 I2C 주소


void SensorRead(unsigned char addr, unsigned char* datbuf, unsigned char cnt) {
  Wire.beginTransmission(ToF250_ADDR);
  Wire.write(byte(addr));
  Wire.endTransmission();
  Wire.requestFrom(ToF250_ADDR, (int)cnt);
  if (cnt <= Wire.available()) {
    *datbuf++ = Wire.read();
    *datbuf = Wire.read();
  }
}


int ReadDistance(int _delay) {
  unsigned short distance;
  unsigned char data_buf[2];
  
  // 모듈의 0x00번지부터 2바이트 읽어오기
  SensorRead(0x00, data_buf, 2);

  // 읽어온 두 바이트를 결합
  distance = data_buf[0] << 8;
  distance |= data_buf[1];

  delay(_delay);
  return distance;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  Serial.println("START");
}

void loop() {
  int x = ReadDistance(50);
  Serial.println(x);
}

