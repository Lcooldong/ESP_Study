#include <Arduino.h>

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <Wire.h>
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 8 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip(NUMPIXELS, DOTSTAR_BRG);

void setup() {

  Serial.begin(115200);
  
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
}

// Runs 10 LEDs at a time along strip, cycling through red, green and blue.
// This requires about 200 mA for all the 'on' pixels + 1 mA per 'off' pixel.

int      head  = 0, tail = -8; // Index of first 'on' and 'off' pixels
uint32_t color = 0x010000;      // 'On' color (starts red) => GRB

void loop() {


  // pimoroni => LED 개수 8개

  strip.setPixelColor(head, color); // 'On' pixel at head
  //strip.setPixelColor(tail, 0);     // 'Off' pixel at tail  마지막 한개 끄기 때문에 tail = -8 하면 안켜짐 - 분석필요
  strip.show();                     // Refresh strip
  delay(20);                        // Pause 20 milliseconds (~50 FPS), 매우 빠르게도 가능(delay(1))

  if(++head >= NUMPIXELS) {         // Increment head index.  Off end of strip?
    head = 0;                       //  Yes, reset head index to start
    if((color >>= 8) == 0)          //  Next color (R->G->B) ... past blue now?
      color = 0x010000;             //   Yes, reset to red
  }
  if(++tail >= NUMPIXELS) tail = 0; // Increment, reset tail index
}



// led 알고리즘 - Adafruit neopixel 참고