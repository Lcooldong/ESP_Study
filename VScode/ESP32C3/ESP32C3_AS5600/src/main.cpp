#include <Arduino.h>

#define RobTillaart
//#define SEEED

#ifdef RobTillaart

#include <Wire.h>
#include <SPI.h>

#include "AS5600.h"
#include "neopixel.h"

// #define SDA_PIN 21
// #define SCL_PIN 22
//#define AS5600_DIR_PIN 17

#define SDA_PIN 7
#define SCL_PIN 8
#define AS5600_DIR_PIN 6




// 참고용 변수 라이브러리에 들어있음
// //  setDirection
// const uint8_t AS5600_CLOCK_WISE         = 0;  //  LOW
// const uint8_t AS5600_COUNTERCLOCK_WISE  = 1;  //  HIGH

// //  0.087890625;
// const float   AS5600_RAW_TO_DEGREES     = 360.0 / 4096;
// //  0.00153398078788564122971808758949;
// const float   AS5600_RAW_TO_RADIANS     = PI * 2.0 / 4096;
// //  4.06901041666666e-6
// const float   AS5600_RAW_TO_RPM         = 1.0 / 4096 / 60;

// //  getAngularSpeed
// const uint8_t AS5600_MODE_DEGREES       = 0;
// const uint8_t AS5600_MODE_RADIANS       = 1;
// const uint8_t AS5600_MODE_RPM           = 2;


MyNeopixel* myNeopxiel = new MyNeopixel();

AS5600 as5600;    // 0x36
//AS5600L as5600; // 0x40

void setup() {
  Serial.begin(115200);
  Wire.setPins(SDA_PIN, SCL_PIN);
  myNeopxiel->InitNeopixel();
  byte count = 0;

  Wire.begin();
  myNeopxiel->pickOneLED(0, myNeopxiel->strip->Color(255, 255, 255), 50, 1);
  // I2C Scan
  for (byte i = 1; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (10);
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");



  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);
  
  as5600.begin(AS5600_DIR_PIN);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  int b = as5600.isConnected();
  Serial.print("Connect: ");
  Serial.println(b);
  delay(1000);

}

void loop() {
  float angle = as5600.rawAngle() * AS5600_RAW_TO_DEGREES;
  Serial.printf("%d %d %lf \r\n",as5600.readAngle(), as5600.rawAngle(), angle);
  int value = map(angle, 0, 359, 0, 255);
  myNeopxiel->pickOneLED(0, myNeopxiel->Wheel((256  + value) & 255), 50, 1);
  // Serial.println(analogRead(4));
  delay(100);
}


#endif


#ifdef SEEED

#include <Wire.h>
#include "AS5600.h"

#define ARDUINO_SAMD_VARIANT_COMPLIANCE

#define SYS_VOL   3.3


String lastResponse;
String noMagnetStr = "Error: magnet not detected";

AMS_5600 ams5600;


void printMenu();
/*******************************************************
/* function: setup
/* In: none
/* Out: none
/* Description: called by system at startup
/*******************************************************/
void setup(){
 Serial.begin(115200);
 Wire.begin();
 printMenu();
}

/*******************************************************
/* function: printMenu
/* In: none
/* Out: none
/* Description: prints menu options and result of last
/* command
/*******************************************************/
void printMenu()
{
  for(int i =0; i<20;i++)
    Serial.println();
  Serial.println("AS5600 Serial Interface Program");
  Serial.println("");
  if(lastResponse.length()>0)
  {
    Serial.println(lastResponse);
    Serial.println("");
  }
  Serial.print("1 - Set start position\t|  "); Serial.println(" 7 - get raw angle");
  Serial.print("2 - Set end position\t|  ");   Serial.println(" 8 - get scaled angle");
  Serial.print("3 - Set max angle range\t|  ");  Serial.println(" 9 - detect magnet");
  Serial.print("4 - Get max angle range\t|  ");  Serial.println("10 - get magnet strength");
  Serial.print("5 - Get start position \t|  ");     Serial.println("11 - get automatic gain conrol");
  Serial.println("6 - get end position \t|  ");
  Serial.println();
  Serial.print("Number of burns remaining: "); Serial.println(String(3 - ams5600.getBurnCount()));
  Serial.println("96 - Burn Angle");
  Serial.println("98 - Burn Settings (one time)");
}

/*******************************************************
/* Function: convertRawAngleToDegrees
/* In: angle data from AMS_5600::getRawAngle
/* Out: human readable degrees as float
/* Description: takes the raw angle and calculates
/* float value in degrees.
/*******************************************************/
float convertRawAngleToDegrees(word newAngle)
{
  /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
  float retVal = newAngle * 0.087890625;
  return retVal;
}

/*******************************************************
/* Function: convertScaledAngleToDegrees
/* In: angle data from AMS_5600::getScaledAngle
/* Out: human readable degrees as float
/* Description: takes the scaled angle and calculates
/* float value in degrees.
/*******************************************************/
float convertScaledAngleToDegrees(word newAngle)
{
  word startPos = ams5600.getStartPosition();
  word endPos = ams5600.getEndPosition();
  word maxAngle = ams5600.getMaxAngle();

  float multipler = 0;

  /* max angle and end position are mutually exclusive*/
  if(maxAngle >0)
  {
    if(startPos == 0)
      multipler = (maxAngle*0.087890625)/4096;
    else  /*startPos is set to something*/
      multipler = ((maxAngle*0.087890625)-(startPos * 0.087890625))/4096;
  }
  else
  {
    if((startPos == 0) && (endPos == 0))
      multipler = 0.087890625;
    else if ((startPos > 0 ) && (endPos == 0))
      multipler = ((360 * 0.087890625) - (startPos * 0.087890625)) / 4096;
    else if ((startPos == 0 ) && (endPos > 0))
      multipler = (endPos*0.087890625) / 4096;
    else if ((startPos > 0 ) && (endPos > 0))
      multipler = ((endPos*0.087890625)-(startPos * 0.087890625))/ 4096;
  }
  return (newAngle * multipler);
}

/*******************************************************
/* Function: burnAngle
/* In: none
/* Out: human readable string of success or failure
/* Description: attempts to burn angle data to AMS5600
/*******************************************************/
String burnAngle()
{
  int burnResult = ams5600.burnAngle();
  String returnStr = "Burn angle error: ";

  switch (burnResult)
  {
    case 1:
      returnStr = "Burn angle success";
      break;
    case -1:
      returnStr += "no magnet detected";
      break;
    case -2:
      returnStr += "no more burns left";
      break;
    case -3:
      returnStr += "no positions set";
      break;
    default:
      returnStr += "unknown";
      break;
  }
  return returnStr;
}

/*******************************************************
/* Function: burnMaxAngleAndConfig
/* In: none
/* Out: human readable string of sucess or failure
/* Description: attempts to burn max angle and config data
/* to AMS5600
/*******************************************************/
String burnMaxAngleAndConfig()
{
  int burnResult = ams5600.burnMaxAngleAndConfig();
  String retStr = "Burn max angle and config error: ";

  switch(burnResult)
  {
    case 1:
      retStr = "Burn max angle and config success";
      break;
    case -1:
      retStr += "chip has been burned once already";
      break;
    case -2:
      retStr += "max angle less than 18 degrees";
      break;
    default:
      retStr += "unknown";
      break;
  }
  return retStr;
}

/*******************************************************
/* Function: loop
/* In: none
/* Out: none
/* Description: main program loop
/*******************************************************/
void loop()
{

  if (Serial.available() > 0)
  {
    char incomingByteBuffer[5] = {0};
    char incomingByte = 0;

    incomingByteBuffer[0];
    incomingByteBuffer[1];

    Serial.readBytes(incomingByteBuffer,2);

    if ((incomingByteBuffer[0] >= 48) && (incomingByteBuffer[0] < 60))
    {
      incomingByte = incomingByteBuffer[0] - 48;
    }

    if ((incomingByteBuffer[1] >= 48) && (incomingByteBuffer[1] < 60))
    {
      incomingByte *=10;
      incomingByte += incomingByteBuffer[1] - 48;
    }


    switch (incomingByte)
    {
      case 1:
      {
        if(ams5600.detectMagnet()==1)
          lastResponse = ("Start angle set to = "+String(convertRawAngleToDegrees(ams5600.setStartPosition()), DEC));  //Print Raw Angle Value
        else
          lastResponse = noMagnetStr;
      }
      break;

      case 2:
      {
        if(ams5600.detectMagnet()==1)
          lastResponse = ("End angle set to = "+String(convertRawAngleToDegrees(ams5600.setEndPosition()), DEC));
        else
          lastResponse = noMagnetStr;
      }
      break;

      case 3:
      {
        if(ams5600.detectMagnet()==1)
          lastResponse = ("Max angle range set to = "+String(convertRawAngleToDegrees(ams5600.setMaxAngle()), DEC));
        else
          lastResponse = noMagnetStr;
      }
      break;

      case 4:
      {
        lastResponse = ("Max angle range= "+String(convertRawAngleToDegrees(ams5600.getMaxAngle()), DEC));
      }
      break;

      case 5:
      {
        lastResponse = ("Start angle = "+String(convertRawAngleToDegrees(ams5600.getStartPosition()), DEC));
      }
      break;

      case 6:
      {
        lastResponse = "End angle = " + String(convertRawAngleToDegrees(ams5600.getEndPosition()),DEC);
      }
      break;

      case 7:
      {
        lastResponse = "Raw angle = "+ String(convertRawAngleToDegrees(ams5600.getRawAngle()),DEC);
      }
      break;

      case 8:
      {
        lastResponse = "Scaled angle = "+String(convertScaledAngleToDegrees(ams5600.getScaledAngle()),DEC);
      }
      break;

      case 9:
      {
        if(ams5600.detectMagnet()==1)
          lastResponse = "Magnet detected";
        else
          lastResponse = noMagnetStr;
      }
      break;

      case 10:
      {
        lastResponse = "Magnet strength ";
        if(ams5600.detectMagnet()==1)
        {
          int magStrength = ams5600.getMagnetStrength();

          if(magStrength == 1)
            lastResponse += "is weak";
          else if(magStrength == 2){
            lastResponse += "is acceptable. ";
            lastResponse += "Current Magnitude: ";
            lastResponse += ams5600.getMagnitude();
          }
          else if (magStrength == 3)
            lastResponse += "is to strong";
        }
        else
          lastResponse = noMagnetStr;
      }
      break;

      case 11:
      {
         lastResponse = "Automatic Gain Control = " + String(ams5600.getAgc(),DEC);
      }
      break;

      case 96:
      {
          lastResponse = burnAngle();
      }
      break;

      case 98:
      {
          lastResponse = burnMaxAngleAndConfig();
      }
      break;

      default:
      {
          lastResponse = "Invalid Entry";
      }
      break;
    }
    /*end of menu processing */
    printMenu();
  }
}





#endif