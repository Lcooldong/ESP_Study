// FONT

/*
FreeMono12pt7b.h    FreeSansBoldOblique12pt7b.h
FreeMono18pt7b.h    FreeSansBoldOblique18pt7b.h
FreeMono24pt7b.h    FreeSansBoldOblique24pt7b.h
FreeMono9pt7b.h     FreeSansBoldOblique9pt7b.h
FreeMonoBold12pt7b.h    FreeSansOblique12pt7b.h
FreeMonoBold18pt7b.h    FreeSansOblique18pt7b.h
FreeMonoBold24pt7b.h    FreeSansOblique24pt7b.h
FreeMonoBold9pt7b.h   FreeSansOblique9pt7b.h
FreeMonoBoldOblique12pt7b.h FreeSerif12pt7b.h
FreeMonoBoldOblique18pt7b.h FreeSerif18pt7b.h
FreeMonoBoldOblique24pt7b.h FreeSerif24pt7b.h
FreeMonoBoldOblique9pt7b.h  FreeSerif9pt7b.h
FreeMonoOblique12pt7b.h   FreeSerifBold12pt7b.h
FreeMonoOblique18pt7b.h   FreeSerifBold18pt7b.h
FreeMonoOblique24pt7b.h   FreeSerifBold24pt7b.h
FreeMonoOblique9pt7b.h    FreeSerifBold9pt7b.h
FreeSans12pt7b.h    FreeSerifBoldItalic12pt7b.h
FreeSans18pt7b.h    FreeSerifBoldItalic18pt7b.h
FreeSans24pt7b.h    FreeSerifBoldItalic24pt7b.h
FreeSans9pt7b.h     FreeSerifBoldItalic9pt7b.h
FreeSansBold12pt7b.h    FreeSerifItalic12pt7b.h
FreeSansBold18pt7b.h    FreeSerifItalic18pt7b.h
FreeSansBold24pt7b.h    FreeSerifItalic24pt7b.h
FreeSansBold9pt7b.h   FreeSerifItalic9pt7b.h
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  delay(2000);

  display.setFont(&FreeSerif9pt7b);
  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(0,20);             
  display.println("Hello, world!");
  display.display();
  delay(2000); 
}
void loop() {
  
}
