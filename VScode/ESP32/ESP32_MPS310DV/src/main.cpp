#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

//PS310D 근접, 이탈 인터럽트 발생 버전
//초기 근접 인터럽트 설정 --> 근접인터럽트 발생시 이탈 인터럽트 설정--> 이탈 인터럽트 발생시 근접 인터럽트 설정--> 반복
//TWS 사용을 가정하여 작성됨

//Device Address
#define PS310D    0x39

//Register Address
#define ENABLE    0x00
#define PTIME     0x02
#define WTIME     0x03
#define PILTL     0x08
#define PILTH     0x09
#define PIHTL     0x0A
#define PIHTH     0x0B
#define PPERS     0x0C
#define CONFIG    0x0D
#define PPULSE    0x0E
#define CONTROL   0x0F
#define PWIDTH    0x10
#define ID        0x12
#define STATUS    0x13
#define T_OSC     0x15
#define T_GAIN    0x16
#define T_OFFSET  0x17
#define PDATAL    0x38   //ADC LOW BYTE, Repeated byte protocol transaction
#define PDATAH    0x39   //ADC HIGH BYTE, Repeated byte protocol transaction
#define TRIM_EN   0x1A
#define INTCLEAR  0x65

#define INTPIN  2   // 인터럽트 핀을 아두이노 D2에 할당

int SensorVal;
int ReadValByteL;
int ReadValByteH;
int Flag = 0;
bool InEarIntActive = true;
bool OutEarIntActive = false;

void checkI2C(int _delay);
void I2C_Write(int DevAddr, int RegAddr, int Val);
int I2C_ReadByte(int DevAddr, int RegAddr);
int I2C_ReadWord(int DevAddr, int RegAddr);
void IntServ();
void IntClr();



void setup() {

  int ChipID;
  int Status;

  Wire.begin();
  Serial.begin(115200);
  delay(100);

   //I2C conntction check
  ChipID = I2C_ReadByte(PS310D, ID);
  if(ChipID != 0x03){
    Serial.println("Check IC Status");
    while(1){

    }
  }

  I2C_ReadByte(PS310D, INTCLEAR);  //Interrupt clear

  ////////Default value setup

  I2C_Write(PS310D, PTIME, 0xFF);
  I2C_Write(PS310D, WTIME, 0xFF);
  I2C_Write(PS310D, PILTL, 0x00);  // Prox Low Threshold Low Byte - 이탈 인터럽트 threshold
  I2C_Write(PS310D, PILTH, 0x00);  // Prox Low Threshold High Byte - 이탈 인터럽트 threshold
  I2C_Write(PS310D, PIHTL, 0x80);  //Interrupt generation over this threshold 0x07D0 = 2000
  I2C_Write(PS310D, PIHTH, 0x00);  //Interrupt generation over this threshold 0x03E8 = 1000
  I2C_Write(PS310D, PPERS, 0x20);  // consecutive values issues interrrups.
  I2C_Write(PS310D, CONFIG, 0x02);
  I2C_Write(PS310D, PPULSE, 0x01);
  I2C_Write(PS310D, CONTROL, 0x80);
  I2C_Write(PS310D, PWIDTH, 0x00);
  I2C_Write(PS310D, T_GAIN, 0x20);   //default value, Gain fine control
  I2C_Write(PS310D, T_OFFSET, 0x20); //default value, offset control
  //I2C_Write(PS310D, T_OSC, 0x20);
  I2C_Write(PS310D, TRIM_EN, 0x03);  //0x03 = offset and gain enable
  I2C_Write(PS310D, ENABLE, 0x2d); // Starting Operation, enable interrupt
  ///////Default value setup

  Serial.println("PS310 Test Setup");

  pinMode(INTPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTPIN), IntServ, FALLING);

}

void loop() {

  SensorVal = I2C_ReadWord(PS310D, PDATAL);
  Serial.printf("Data : %d\r\n" ,SensorVal);
  // Serial.println("========================");
  delay(10);

  if(Flag == 1) {
    if(InEarIntActive == true){ //when in ear is detected
      I2C_Write(PS310D, PILTL, 0x28);  //Interrupt generation under 0x03E8 = 1000
      I2C_Write(PS310D, PILTH, 0x00);
      I2C_Write(PS310D, PIHTL, 0xFF);
      I2C_Write(PS310D, PIHTH, 0xFF);
      InEarIntActive = false;
      Serial.println("======================== sensor is in EAR");
    } else { //when out ear is detected
      I2C_Write(PS310D, PILTL, 0x00);
      I2C_Write(PS310D, PILTH, 0x00);
      I2C_Write(PS310D, PIHTL, 0x80);  //Interrupt generation over this threshold 0x07D0 = 2000
      I2C_Write(PS310D, PIHTH, 0x00);
      InEarIntActive = true;
      Serial.println("======================== sensor is out of EAR");
    }

    Flag = 0;
    delay(50);
    IntClr();
    Serial.println("PS310 Test");


    // Serial.println(I2C_ReadByte(PS310D, STATUS));

  }
    //delay(10);

}

void I2C_Write(int DevAddr, int RegAddr, int Val) {
  Wire.beginTransmission(DevAddr);
  Wire.write(RegAddr);
  Wire.write(Val);
  Wire.endTransmission();
}

int I2C_ReadByte(int DevAddr, int RegAddr) {
  int ReadVal;
  Wire.beginTransmission(DevAddr);
  Wire.write(RegAddr);
  Wire.endTransmission();
  Wire.requestFrom(DevAddr,1);
  ReadVal = Wire.read();
  //Wire.endTransmission();
  return(ReadVal);
 }

int I2C_ReadWord(int DevAddr, int RegAddr) {
  int ReadWord;
  int ReadByteL;
  int ReadByteH;
  Wire.beginTransmission(DevAddr);
  Wire.write(RegAddr);
  Wire.endTransmission();
  if (Wire.requestFrom(DevAddr,2)>=2){
    ReadByteL = Wire.read();
    ReadByteH = Wire.read();
  }
  ReadWord = ReadByteL | (ReadByteH << 8);
  return(ReadWord);
 }

void IntServ() {   //Interrupt service routine
    Flag = 1;
    //delay(10);

 }

void IntClr() {
      while(I2C_ReadByte(PS310D, STATUS)== 34){
        I2C_ReadByte(PS310D, INTCLEAR);
        I2C_ReadByte(PS310D, STATUS);
   }
}



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
