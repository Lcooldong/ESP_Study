// A non-blocking everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define ESP32


// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    25

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 144

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

unsigned long pixelPrevious = 0;        // Previous Pixel Millis
unsigned long patternPrevious = 0;      // Previous Pattern Millis
int           patternCurrent = 0;       // Current Pattern Number
int           patternInterval = 5000;   // Pattern Interval (ms)
int           pixelInterval = 50;       // Pixel Interval (ms)
int           pixelQueue = 0;           // Pattern Pixel Queue
int           pixelCycle = 0;           // Pattern Pixel Cycle
uint16_t      pixelCurrent = 0;         // Pattern Current Pixel Number
uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels
long randNumber;

// setup() function -- runs once at startup --------------------------------
void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(5); // Set BRIGHTNESS to about 1/5 (max = 255)
}

// loop() function -- runs repeatedly as long as board is on ---------------
void loop() {
//  rainbow(10);
//  gauge_rainbow_Wipe2(5, 1);
  gauge_rainbow_Wipe(36, 1);
  unsigned long currentMillis = millis();                     //  Update current time
//  if((currentMillis - patternPrevious) >= patternInterval) {  //  Check for expired time
//    patternPrevious = currentMillis;
//    patternCurrent++;                                         //  Advance to next pattern
//    if(patternCurrent >= 7)
//      patternCurrent = 0;
//  }
//
//  
//  
  if(currentMillis - pixelPrevious >= pixelInterval) {        //  Check for expired time
    pixelPrevious = currentMillis;                            //  Run current frame
//    rainbow(1);
//    colorWipe(strip.Color(0, 0, 255), 1); // Blue
//    gauge_rainbow_Wipe2(1, 1);
//    rainbow_delay(1);
//    gauge_Wipe(strip.Color(255, 43, 123), 1);
//    switch (patternCurrent) {
//      case 7:
//        theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
//        break;
//      case 6:
//        rainbow(10); // Flowing rainbow cycle along the whole strip
//        break;     
//      case 5:
//        theaterChase(strip.Color(0, 0, 127), 50); // Blue
//        break;
//      case 4:
//        theaterChase(strip.Color(127, 0, 0), 50); // Red
//        break;
//      case 3:
//        theaterChase(strip.Color(127, 127, 127), 50); // White
//        break;
//      case 2:
//        colorWipe(strip.Color(0, 0, 255), 50); // Blue
//        break;
//      case 1:
//        colorWipe(strip.Color(0, 255, 0), 50); // Green
//        break;        
//      default:
//        colorWipe(strip.Color(255, 0, 0), 50); // Red
//        break;
//    }
  }
}

// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.

void gauge_rainbow_Wipe2(int cycle_speed , int wait){
  if(pixelInterval != wait)
    pixelInterval = wait;
    
  randNumber = random(0, LED_COUNT); // 끝 제외  144개 0~143
  for(uint16_t i=0; i < pixelNumber; i++) {
    strip.setPixelColor(i, Wheel((i + pixelCycle) & 255)); //  Update delay time  
    strip.setPixelColor(randNumber + i, strip.Color(0, 0, 0));
    strip.show();
  }
  
  
  pixelCurrent = randNumber;
  pixelCycle = pixelCycle + cycle_speed;
  pixelQueue = LED_COUNT - pixelCurrent;
  if(pixelCycle >= 256)
    pixelCycle = 0;
  if(pixelQueue >= LED_COUNT)
    pixelQueue = 0;
  
}


void sparkle(int cycle_speed , int wait){
  if(pixelInterval != wait)
    pixelInterval = wait;
    
  randNumber = random(0, LED_COUNT); // 끝 제외  144개 0~143

  strip.setPixelColor(pixelCurrent, Wheel((pixelCurrent + pixelCycle) & 255)); //  Update delay time
  strip.show();
  
  pixelCurrent = randNumber;
  pixelCycle = pixelCycle + cycle_speed;
  if(pixelCycle >= 256)
    pixelCycle = 0;                             
}

void gauge_rainbow_Wipe(int cycle_speed , int wait){
  if(pixelInterval != wait)
    pixelInterval = wait;
  randNumber = random(0, LED_COUNT); // 끝 제외  144개 0~143

  if(pixelCurrent > randNumber){
    for(int i = pixelCurrent; i >=randNumber; i-- ){
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show();
    }
  }else{
    for(uint16_t j=0; j<=randNumber; j++){
      strip.setPixelColor(j, Wheel((j + pixelCycle) & 255));
      strip.show();
    }
  }

  pixelCurrent = randNumber;
  pixelCycle = pixelCycle + cycle_speed;
  pixelQueue++;
  if(pixelCycle >= 256)
    pixelCycle = 0;                             
  if(pixelQueue >= LED_COUNT)
    pixelQueue = 0;    
  
}

void gauge_Wipe(uint32_t color, int wait){
  if(pixelInterval != wait)
    pixelInterval = wait;
  randNumber = random(0, LED_COUNT); // 끝 제외  144개 0~143
  if(pixelCurrent > randNumber){
    for(int i=pixelCurrent; i >=randNumber; i-- ){
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show();
    }
  }else{   
    for(int i=pixelCurrent; i<=randNumber; i++){   
      strip.setPixelColor(i, color);
      strip.show();   
    }
  }
  
  
  pixelCurrent = randNumber;
}

void colorWipe(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  strip.setPixelColor(pixelCurrent, color); //  Set pixel's color (in RAM)
  strip.show();                             //  Update strip to match
  pixelCurrent++;                           //  Advance current pixel
  if(pixelCurrent >= pixelNumber)           //  Loop the pattern from the first LED
    pixelCurrent = 0;
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  for(int i = 0; i < pixelNumber; i++) {
    strip.setPixelColor(i + pixelQueue, color); //  Set pixel's color (in RAM)
  }
  strip.show();                             //  Update strip to match
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Set pixel's color (in RAM)
  }
  pixelQueue++;                             //  Advance current pixel
  if(pixelQueue >= 3)
    pixelQueue = 0;                         //  Loop the pattern from the first LED
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   
  for(uint16_t i=0; i < pixelNumber; i++) {
    strip.setPixelColor(i, Wheel((i + pixelCycle) & 255)); //  Update delay time  
  }
  strip.show();                             //  Update strip to match
  pixelCycle++;                             //  Advance current cycle
  if(pixelCycle >= 256)
    pixelCycle = 0;                         //  Loop the cycle back to the begining
}

void rainbow_delay(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}



//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time  
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, Wheel((i + pixelCycle) % 255)); //  Update delay time  
  }
  strip.show();
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Update delay time  
  }      
  pixelQueue++;                           //  Advance current queue  
  pixelCycle++;                           //  Advance current cycle
  if(pixelQueue >= 3)
    pixelQueue = 0;                       //  Loop
  if(pixelCycle >= 256)
    pixelCycle = 0;                       //  Loop
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
