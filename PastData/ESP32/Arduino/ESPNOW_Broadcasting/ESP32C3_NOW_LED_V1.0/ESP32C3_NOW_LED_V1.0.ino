// board : https://github.com/01Space/ESP32-C3-0.42LCD
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#define INFO_SCREEN_DELAY 3000
#define SDA_PIN 5
#define SCL_PIN 6
#define LED_PIN 2
#define LED_COUNT 60
#define CHANNEL 1



#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.42" OLED


///////////////////////////////////////////////////////////////
String success;
int device_id = 1;    // check

////////////////////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct _packet
{
    uint8_t device_led;
    uint8_t RED;  // 데이터 타입 : 1
    uint8_t GREEN;
    uint8_t BLUE;
    uint8_t brightness;
    uint8_t style;            // 데이터 : 4 
    uint8_t wait;
    uint8_t checksum;  // 체크섬 : 2
}PACKET;
#pragma pack(pop)

typedef enum {
  oneColor = 1,
  CHASE,
  RAINBOW,
  COLORWIPE,
  CHASE_RAINBOW
}STYLE_Typedef;

PACKET serial_data = {0, };
PACKET incomingReadings;

esp_now_peer_info_t peerInfo;
/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  init_Neopixel(50);
  WiFi.mode(WIFI_AP_STA);
  configDeviceAP();
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
//  WiFi.disconnect();
  // Init ESP-NOW
  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  } 

//  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
 
}

void loop() {  
  rainbow(20);
}


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  uint8_t target_board_led = incomingReadings.device_led;
  uint8_t R = incomingReadings.RED;
  uint8_t G = incomingReadings.GREEN;
  uint8_t B = incomingReadings.BLUE;
  uint8_t _brightness = incomingReadings.brightness;
  uint8_t waitORtimes = incomingReadings.wait;
  strip.setBrightness(_brightness);
  
  // target_board_led >> 4
  if(device_id == (target_board_led / 16))
  {
    switch(incomingReadings.style)
    {
      case oneColor:
        pickOneLED(target_board_led%16, strip.Color(R, G, B), _brightness, waitORtimes);
        break;
        
      case CHASE:
        theaterChase(strip.Color(R, G, B), waitORtimes);
        resetNeopixel();
        break;
        
      case RAINBOW:
        rainbow(waitORtimes);
        resetNeopixel();
        break;
        
      case COLORWIPE:
        colorWipe(strip.Color(R, G, B), waitORtimes * 10);
        break;
      case CHASE_RAINBOW:
        theaterChaseRainbow(waitORtimes);
        resetNeopixel();
        break;

      default:
        resetNeopixel();
        break;
    }
  } 
}

////////////////////////////////////////////////////////////////////////////
// config AP SSID
void configDeviceAP() {
  const char *SSID = "RemoteLED_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}


/////////////////////////////////////////////////////////////////////////////

void init_Neopixel(uint8_t brightness){
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void pickOneLED(uint8_t ledNum, uint32_t color, uint8_t brightness, int wait){
    strip.setBrightness(brightness);
    strip.setPixelColor(ledNum, color);  
    strip.show();                                               
    delay(wait);
}

void blinkNeopixel(uint32_t color, int times, int delays){
  for(int i = 0; i < times; i++){
    pickOneLED(0, color, 50, delays);
    pickOneLED(0, strip.Color(0, 0, 0), 0, delays);
  }
}

void resetNeopixel(){
  for(int i=0; i < 256; i++){
    pickOneLED(i, strip.Color(0, 0, 0), 0, 0 );
  } 
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
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
