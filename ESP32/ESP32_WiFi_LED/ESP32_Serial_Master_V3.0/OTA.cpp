#include "OTA.h"
#include "Arduino.h"


//#define ESP32
#define ESP32_RTOS

#define LED_PIN 23
#define LED_COUNT 1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


unsigned long connectingTime = 0;
unsigned long connecting_interval = 10000;
int WiFiManger_flag = 0;

unsigned long previousMillis = 0;
unsigned long interval = 30000;
int connection_flag = 0;

char TEMP_SSID[32] = {0,};
char TEMP_PW[32] = {0, };

char EEPROM_SSID[32];
char EEPROM_PW[32];


//extern int neopixel_Flag;

void init_Neopixel(uint8_t brightness){
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
}

#if defined(ESP32_RTOS) && defined(ESP32)
void ota_handle( void * parameter ) {
  for (;;) {
    ArduinoOTA.handle();
    delay(3500);
  }
}
#endif

void setupOTA() {
  initWiFi();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Connection Test -> /update to upload bin file");
  });
  
  AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
  server.begin();
  Serial.println("HTTP server started");
  
  
  TelnetStream.begin();
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  Serial.println("--------Saved Data--------");
  readWiFiEEPROM();
  Serial.println("--------------------------");
  WiFi.begin(EEPROM_SSID, EEPROM_PW);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.print('.');
    //delay(1000);
    blinkNeopixel(0, 255, 0, 2);
    connecting_interval = millis();
    if(connecting_interval >= connecting_interval)
    {
      Serial.println();
      Serial.println("Not Connected -> Open WiFi Manager");
      pickOneLED(0, 255, 0, 0, 50, 50);
      WiFiManger_flag = 1;
      break;
    }
  }
  if(WiFiManger_flag == 1)
  {
    wm.resetSettings();
    bool res;
    res = wm.autoConnect("RemoteLED");
    
    if(!res) 
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else 
    {
        String SSID_NAME = WiFiManager().getWiFiSSID();
        String PW_NAME = WiFiManager().getWiFiPass();
        Serial.print("SSID : ");
        Serial.println(SSID_NAME);
        
        Serial.print("PASSWORD : ");
        Serial.println(PW_NAME);
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        blinkNeopixel(0, 0, 255, 2);
        SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
        PW_NAME.toCharArray(TEMP_PW, sizeof(TEMP_PW));
        writeWiFiEEPROM();
        ESP.restart();
    }
  }
  else 
  {
    Serial.println();
    Serial.print("connected ->");
    Serial.println(WiFi.localIP());
    showOLED_IP_Address();
    pickOneLED(0, 0,   0,   255, 50, 50);
    
  }
  readWiFiEEPROM();
  
}

void reconnectWiFi(){
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.println(millis());
    Serial.println("Reconnecting to WiFi...");
    
    WiFi.disconnect();
    WiFi.reconnect();
    connection_flag = 1;
    previousMillis = currentMillis;
  }
  else if(connection_flag == 1)
  {
    Serial.println("reconnected");  
    connection_flag = 0;
  }
}


void writeWiFiEEPROM(){
    for(int i=0; i < sizeof(EEPROM_SSID); i++)
    {
      EEPROM.write(i, 0);
      EEPROM.write(i + sizeof(EEPROM_SSID), 0);
      EEPROM.write(i, TEMP_SSID[i]);
      EEPROM.write(i + sizeof(EEPROM_SSID), TEMP_PW[i]);
   }
    EEPROM.commit();
}

void readWiFiEEPROM(){
    for(int j=0; j< sizeof(EEPROM_SSID); j++)
    {
      EEPROM_SSID[j] = EEPROM.read(j);
      EEPROM_PW[j] = EEPROM.read(j + sizeof(EEPROM_SSID));
    }
    Serial.print("EEPROM SSID : ");
    Serial.println(EEPROM_SSID);
    TelnetStream.print("EEPROM SSID : ");
    TelnetStream.println(EEPROM_SSID);
    Serial.print("EEPROM SSID : ");
    Serial.println(EEPROM_PW);
    
    TelnetStream.print("EEPROM SSID : ");
    TelnetStream.println(EEPROM_PW);
    
    display.setCursor(0, 30);
    display.println(EEPROM_SSID);
    display.display();
    display.setCursor(0, 40);
    display.println(EEPROM_PW);
    display.display();
}





void initOLED(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  // Display static text
  display.println("Hello, world!");
  display.display(); 
}


void showOLED_IP_Address(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 15);
  display.println(WiFi.localIP());

//  display.setCursor(0, 30);
//  display.println(EEPROM_SSID);
//  display.display();
//  display.setCursor(0, 40);
//  display.println(EEPROM_PW);
//  display.display();
  

}





void pickOneLED(uint8_t ledNum, uint8_t R, uint8_t G, uint8_t B, uint8_t brightness, uint8_t wait){
    strip.setBrightness(brightness);
    strip.setPixelColor(ledNum, strip.Color(R, G, B));  
    strip.show();                                               
    delay(wait);
}

void blinkNeopixel(uint8_t R, uint8_t G, uint8_t B, int times){
  for(int i = 0; i < times; i++){
    pickOneLED(0, R, G, B, 50, 500);
    pickOneLED(0, R, G, B, 50, 500);
  }
}


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
//    if(neopixel_Flag == 1) break;
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
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

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
