#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>

#include "Button.h"

#define OLED_128X64_ADDRESS 0x3C

const int RECV_PIN = 25;
const int SEND_PIN = 26;
const int BUTTON_PIN = 27;

IRrecv irrecv(RECV_PIN);
IRsend irsend(SEND_PIN);
decode_results results;

unsigned long countTime = 0;
unsigned long count = 0;

unsigned long irTime = 0;


const uint32_t eighteenDegree = 0x880831C;
const uint32_t thirtyDegree = 0x8808F18;
uint32_t currentDegree = 0x880831C;

uint8_t currentIndex = 0;
uint32_t degrees[13] = {
  0x880831C,
  0x880841D,
  0x880851E,
  0x880861F,
  0x8808710,
  0x8808811,
  0x8808912,
  0x8808A13,
  0x8808B14,
  0x8808C15,
  0x8808D16,
  0x8808E17,
  0x8808F18
};



uint16_t* lastRecv;
uint16_t* lastBuf;
uint16_t lastLen;
uint64_t lastValue;
uint16_t lastBits;

int powerToggle = 0;
bool oledState = false;

decode_results lastResults;
Button myBtn(BUTTON_PIN, false, 20); // Pull up -> true

const int SDA_PIN = 21;
const int SCL_PIN = 22;
// U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE);



void get_IR_Data();
void localSwitch();
void myCommand();
void connectOLED(uint8_t address);
void checkI2C(int _delay);

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.setFontDirection(1);
  u8g2.clearBuffer();
  u8g2.setCursor(100, 10);
  u8g2.printf("Air RC");
  u8g2.setCursor(80, 10);
  u8g2.printf("--Start--");
  u8g2.sendBuffer();

  irrecv.enableIRIn(); // Start the receiver
  irsend.begin();
  Serial.println("Enabled IRin");
  Serial.printf("Value : %d\r\n", sizeof(degrees)/sizeof(uint32_t));
}

void loop() {

  get_IR_Data();
  localSwitch();
  myCommand();



  if(millis() - countTime > 1000)
  {
    countTime = millis();
    // Serial.printf("CNT : %d\r\n", count++);
    // connectOLED(OLED_128X64_ADDRESS);
    // checkI2C(2000);
  }




  // int btnRead = digitalRead(BUTTON_PIN);
  // if(!btnRead)
  // {
  //   Serial.println("Button Read");
  //   Serial.printf("BTN[%d] \r\n", lastResults.rawlen);
  //   irsend.sendRaw((uint16_t*)lastResults.rawbuf, lastResults.rawlen, 38);
  //   switch (lastResults.decode_type)
  //   {
  //   case SAMSUNG:
  //     irsend.sendSAMSUNG(lastResults.value, lastResults.bits, 2);
  //     Serial.println("Send SAMGSUNG");
  //     break;
  //   case LG:
  //     irsend.sendLG(lastResults.value, lastResults.bits, 2);
  //     Serial.println("Send LG");
  //     break;  
  //   default:
  //     break;
  //   }
  //   delay(500);
  // }

 
}





void get_IR_Data()
{
  if(millis() - irTime > 100)
  {
    
    if (irrecv.decode(&results)) {
      // Serial.printf( "Received : 0x%X \r\n",  results.value); // Only One can read
      serialPrintUint64(results.value, HEX);
      Serial.println("");

      u8g2.setCursor(40, 4);
      u8g2.print(results.value, HEX);
      u8g2.sendBuffer();

      lastResults = results;

      Serial.printf("Bits : %d | address : %d  | %X\r\n", results.bits, results.decode_type, results.state);


      switch (lastResults.decode_type)
      {
      case SAMSUNG:
        Serial.println("SAMSUNG");

        for (int i = 0; i < lastResults.rawlen; i++)
        {
          
          Serial.printf("%4d |", lastResults.rawbuf[i]);
        }
        Serial.println("\r\n------------------------------");
        break;
      case LG:
        Serial.println("LG");
        break;
      default:
        break;
      }
   

      irrecv.resume(); // Receive the next value
    } 

    irTime = millis();
  }

}




void localSwitch()
{

  myBtn.read();

  if(myBtn.pressedFor(3000,5000))
  {
    Serial.printf("BUTTON PressedFor 3000\r\n");
    // ESP.restart();
  } 
  else if(myBtn.wasReleasefor(1000))
  {
    Serial.printf("BUTTON Released  1000\r\n"); // Button Pressing Filter

  } 
  else if (myBtn.wasReleased()) 
  {
      Serial.printf("BUTTON Pushed \r\n");

      if(!powerToggle)
      {
        irsend.sendLG(0x88C0051, 28, 2);
        powerToggle = 1;
        Serial.println("Button -> ON");

        u8g2.clearBuffer();
        u8g2.setCursor(100, 10);
        u8g2.printf("Air RC");
        u8g2.setCursor(80, 10);
        u8g2.printf("--Start--");
        u8g2.setCursor(60, 10);
        u8g2.printf("ON");
        u8g2.sendBuffer();
        
      }
      else
      {
        irsend.sendLG(0x880190A, 28, 2); 
        powerToggle = 0;
        Serial.println("Button -> OFF");
        u8g2.clearBuffer();
        u8g2.setCursor(100, 10);
        u8g2.printf("Air RC");
        u8g2.setCursor(80, 10);
        u8g2.printf("--Start--");
        u8g2.setCursor(60, 10);
        u8g2.printf("OFF");
        u8g2.sendBuffer();
      }
  }
}



void myCommand()
{
   if(Serial.available())
  {
    char c = Serial.read();
    switch (c)
    {
    case '1':
    {
      irsend.sendLG2(0x88C0051, 28, 2);
      Serial.printf("Send 1 - Turn Off\r\n");
      break;
    }
    
    case '2':
      irsend.sendLG2(0x880190A, 28, 2);
      Serial.printf("Send 2 - Recent Turn On\r\n");
      break;
    case '3':
      irsend.sendLG2(0x8801C0D, 28, 2);
      Serial.printf("Send 3 - Turn On\r\n");
      break;
    case '4':
      irsend.sendLG2(0x8809902, 28, 2);
      Serial.printf("Send 4 - Remove Humidity\r\n");
      break;
    case '5':
      irsend.sendLG2(0x8808A13, 28, 2);
      Serial.printf("Send 4 - Cooling Default 25'c\r\n");
      break;
    case 'u':
      if( currentDegree <= thirtyDegree)
      {
        if(currentIndex < sizeof(degrees)/sizeof(uint32_t))
        {
          currentIndex++;
        currentDegree = degrees[currentIndex];
        }
      }

      irsend.sendLG2(currentDegree, 28, 2);
      Serial.printf("Send Up - Cooling 0x%X\r\n", currentDegree);
      break;
    case 'd':
      if( currentDegree >= eighteenDegree)
      {
        if(currentIndex >0)
        {
          currentIndex--;
          currentDegree = degrees[currentIndex];
        }      
      }

      irsend.sendLG2(currentDegree, 28, 2);
      Serial.printf("Send Down - Cooling 0x%X\r\n", currentDegree);
      break;
    case 't':
      break;

    default:
      break;
    }
  }

}


void connectOLED(uint8_t address){
  Wire.begin();
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  if (error == 0) 
  {
    // Serial.println("OLED Connected");
    if(!oledState)
    {
      u8g2.begin();
      oledState = true;
    }
  }
  else
  {
    // Serial.println("OLED Disconnected");
    oledState = false;    
  }
  Wire.end();
}

void checkI2C(int _delay){
  // delay(1000);
  Wire.begin();
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
  Wire.end();
  delay(_delay);
}