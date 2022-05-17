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


extern int neopixel_Flag;

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

void setupOTA(const char* nameprefix) {
  // Configure the hostname
  uint16_t maxlen = strlen(nameprefix) + 7;
  char *fullhostname = new char[maxlen];
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(fullhostname, maxlen, "%s-%02x%02x%02x", nameprefix, mac[3], mac[4], mac[5]);
  ArduinoOTA.setHostname(fullhostname);
  delete[] fullhostname;

  // Configure and start the WiFi station
  initWiFi();

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232); // Use 8266 port if you are working in Sloeber IDE, it is fixed there and not adjustable


  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
  //NOTE: make .detach() here for all functions called by Ticker.h library - not to interrupt transfer process in any way.

    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("\nAuth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("\nBegin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("\nConnect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("\nReceive Failed");
    else if (error == OTA_END_ERROR) Serial.println("\nEnd Failed");
  });

  ArduinoOTA.begin();
  TelnetStream.begin();

  Serial.println("OTA Initialized");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


#if defined(ESP32_RTOS) && defined(ESP32)
  xTaskCreate(
    ota_handle,          /* Task function. */
    "OTA_HANDLE",        /* String with name of task. */
    10000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL);            /* Task handle. */
#endif
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
    pickOneLED(0, 255, 0, 0, 400);
    pickOneLED(0, 0, 0, 0, 200);
    pickOneLED(0, 255, 0, 0, 400);
    pickOneLED(0, 0, 0, 0, 200);
    connecting_interval = millis();
    if(connecting_interval >= connecting_interval)
    {
      Serial.println();
      Serial.println("Not Connected -> Open WiFi Manager");
      pickOneLED(0, 255, 0, 0, 50);
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
        pickOneLED(0, 0,   0,   255, 200);
        pickOneLED(0, 0,   0,   0, 200);
        pickOneLED(0, 0,   0,   255, 200);
        pickOneLED(0, 0,   0,   0, 200);
        pickOneLED(0, 0,   0,   255, 50);
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
    pickOneLED(0, 0,   0,   255, 50);
    
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





void pickOneLED(uint8_t ledNum, uint8_t R, uint8_t G, uint8_t B, int wait){
    strip.setPixelColor(ledNum, strip.Color(R, G, B));         
    strip.show();                                               
    delay(wait);
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
    if(neopixel_Flag == 1) break;
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
