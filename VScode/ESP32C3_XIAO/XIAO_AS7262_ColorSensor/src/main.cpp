#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AS726x.h>
#include "neopixel.h"

//#define DEBUG
#define SENSOR_MAX 5000

#define BLACK   0x0000
#define GRAY    0x8410
#define WHITE   0xFFFF
#define RED     0xF800
#define ORANGE  0xFA60
#define YELLOW  0xFFE0  
#define LIME    0x07FF
#define GREEN   0x07E0
#define CYAN    0x07FF
#define AQUA    0x04FF
#define BLUE    0x001F
#define MAGENTA 0xF81F
#define PINK    0xF8FF

const int AS7262_LED_PIN = D3;
const int LED_CHANNEL_0 = 0;

Adafruit_AS726x ams;
MyNeopixel* pixel_1 = new MyNeopixel(20, D0);

uint16_t sensorValues[AS726x_NUM_CHANNELS];
float calibratedValues[AS726x_NUM_CHANNELS];
int calibratedValuesInt[AS726x_NUM_CHANNELS];
uint8_t color8bit[AS726x_NUM_CHANNELS];
int rgbCalValues[3];
int rgbValues[3];
int colorResults[AS726x_NUM_CHANNELS];

uint16_t colors[] = {
  MAGENTA,
  BLUE,
  GREEN,
  YELLOW,
  ORANGE,
  RED
};

uint64_t colorTime = 0;

void bubbleSort(int data[], int n);

void setup() {
  Serial.begin(115200);
  // while(!Serial);

  if(!ams.begin()){
    Serial.println("could not connect to sensor! Please check your wiring.");
    while(1);
  }

  ledcSetup(LED_CHANNEL_0, 5000, 8);
  ledcAttachPin(AS7262_LED_PIN, LED_CHANNEL_0);
  ledcWrite(LED_CHANNEL_0, 200);  // 0 is On

  pixel_1->InitNeopixel();
  // ams.drvOn();
  ams.setConversionType(MODE_2);
}

void loop() {

  
  // if(millis() - colorTime > 100)
  // {
    
  //   colorTime = millis();
  // }


  
  
  // ams.startMeasurement();

  if(ams.dataReady())
  {
    uint8_t temp = ams.readTemperature();
    ams.readRawValues(sensorValues);
    ams.readCalibratedValues(calibratedValues);

    // RGB
    rgbCalValues[0] = (int)calibratedValues[5];
    rgbCalValues[1] = (int)calibratedValues[2];
    rgbCalValues[2] = (int)calibratedValues[1];

    for (int i = 0; i < AS726x_NUM_CHANNELS; i++)
    {
      calibratedValuesInt[i] = calibratedValues[i];
      colorResults[i] = calibratedValuesInt[i];
    }
    
    bubbleSort(colorResults, AS726x_NUM_CHANNELS);
    bubbleSort(rgbCalValues, 3);
 #ifdef DEBUG   
    Serial.print("Calibrated : ");
#endif
    for (int i = 0; i < AS726x_NUM_CHANNELS; i++)
    {
      color8bit[i] = map(calibratedValues[i], 0, rgbCalValues[2], 0, 255);
#ifdef DEBUG
      Serial.printf("%4.0f | ", calibratedValues[i]);
#endif
      if(i == 5)
      {
        for (int j = 0; j < 20; j++)
        {
          pixel_1->pickOneLED(j, pixel_1->strip->Color(color8bit[5], color8bit[2], color8bit[1]), 20, 0);            
        }
        Serial.printf("MAX[%d] -> [ ", colorResults[5]);
        for (int i = 0; i < AS726x_NUM_CHANNELS; i++)
        {
          Serial.printf("%d , ", calibratedValuesInt[i]);
        }
        Serial.printf("] =>");
        
        if(colorResults[5] == calibratedValuesInt[0])
        {
          Serial.println("Violet");
        }
        else if(colorResults[5] == calibratedValuesInt[1])
        {
          Serial.println("Green");
        }
        else if(colorResults[5] == calibratedValuesInt[2])
        {
          Serial.println("Blue");
        }
        else if(colorResults[5] == calibratedValuesInt[3])
        {
          Serial.println("Yellow");
        }
        else if(colorResults[5] == calibratedValuesInt[4])
        {
          Serial.println("Orange");
        }
        else if(colorResults[5] == calibratedValuesInt[5])
        {
          Serial.println("Red");
        }

        // if(rgbCalValues[2] == calibratedValuesInt[5])
        // {
        //   Serial.println("Red");
        // }
        // else if(rgbCalValues[2] == calibratedValuesInt[2])
        // {
        //   Serial.println("Green");
        // }
        // else if(rgbCalValues[2] == calibratedValuesInt[1])
        // {
        //   Serial.println("Blue");
        // }
        // else
        // {

        // }
      }

    }

#ifdef DEBUG
    Serial.println();

    Serial.print("8bits      : ");
    for (int i = 0; i < AS726x_NUM_CHANNELS; i++)
    {
      Serial.printf("%4d | ", color8bit[i]);
    }
    Serial.println();


    Serial.print("RAW        : ");
    for (int i = 0; i < AS726x_NUM_CHANNELS; i++)
    {
      Serial.printf("%4d | ", sensorValues[i]);
    }
    Serial.println();  
  
#endif
    // Serial.print(">Temp:  "); Serial.println(temp);
    // Serial.print(">Violet:"); Serial.println(sensorValues[AS726x_VIOLET]);
    // Serial.print(">Blue:  "); Serial.println(sensorValues[AS726x_BLUE]);
    // Serial.print(">Green: "); Serial.println(sensorValues[AS726x_GREEN]);
    // Serial.print(">Yellow:"); Serial.println(sensorValues[AS726x_YELLOW]);
    // Serial.print(">Orange:"); Serial.println(sensorValues[AS726x_ORANGE]);
    // Serial.print(">Red:   "); Serial.println(sensorValues[AS726x_RED]);
    // Serial.println();
  }


  

}

void bubbleSort(int data[], int n)
{
  // int i, j;
  int temp;
  int flag = 1;

  for (int i = n-1; i > 0 && flag ; i--)
  {
    flag = 0;
    for (int j = 0; j < i; j++)
    {
      if(data[j] > data[j + 1])
      {
        temp = data[j];
        data[j] = data[j+1];
        data[j+1] = temp;
        flag = 1;
      }
    }
  }
}