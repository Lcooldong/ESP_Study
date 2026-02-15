#define SDA_PIN 5
#define SCL_PIN 6

#include <esp_now.h>
#include <AsyncTCP.h>
#include <TelnetStream.h>
#include <WiFiManager.h>
//#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Adafruit_NeoPixel.h>
#include "SPIFFS.h"
#include <U8g2lib.h>
#include <Wire.h>

unsigned long RESET_TIME = 1000*60*60*24; // 1 day
volatile int count0;  //extern OTA.cpp
volatile int count1;  //extern OTA.cpp
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

String success;
uint8_t incomingRGB[3];

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

int neopixel_Flag = 0;
#define LED_PIN 4
#define LED_COUNT 6
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);    // circle 8, strip 8
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.42" OLED

esp_now_peer_info_t peerInfo;


char ssid[32]          = "YOUR_SSID";
char pass[32]          = "YOUR_PASS";
float timeZone       = 9;
uint8_t summerTime   = 0; // 3600

unsigned long connectingTime = 0;
unsigned long connecting_interval = 10000;
int WiFiManager_flag = 0;
int connection_flag = 0;
extern int neopixel_Flag;

extern char ssid[32];
extern char pass[32];
extern float timeZone;
extern uint8_t summerTime;


char TEMP_SSID[32] = {0,};
char TEMP_PASS[32] = {0, };
char WiFiManager_Name[32];

extern volatile int count0;
extern volatile int count1; 
extern portMUX_TYPE timerMux;
int blinkToggle = 0;


bool FORMAT_SPIFFS_IF_FAILED = true;
AsyncWebServer server(80);

//////////////////////////////////////////////////////////
int device_id = 1;    // check
/////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
   
  initWiFi();
  // Init ESP-NOW
  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Connection Test -> /update to upload bin file");
  });
  AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
  server.begin();
  Serial.println("HTTP server started");


  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  timer_init();

  
}

void loop() {
  reconnectWiFi();
  
  
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
  Serial.print("Bytes received: ");
  Serial.println(len);
  TelnetStream.print("Bytes received: ");
  TelnetStream.println(len);
  
  incomingRGB[0] = incomingReadings.RED;
  incomingRGB[1] = incomingReadings.GREEN;
  incomingRGB[2] = incomingReadings.BLUE;
  
  Serial.print("Device_LED : ");
  Serial.println(incomingReadings.device_led);

  Serial.print("Red : ");
  Serial.println(incomingRGB[0]);
  Serial.print("Green : ");
  Serial.println(incomingRGB[1]);
  Serial.print("Blue : ");
  Serial.println(incomingRGB[2]);
  Serial.print("Brightness : ");
  Serial.println(incomingReadings.brightness);
  Serial.print("Sytle : ");
  Serial.println(incomingReadings.style);
  Serial.print("wait Time : ");
  Serial.println(incomingReadings.wait);
  Serial.print("checksum : ");
  Serial.println(incomingReadings.checksum);

  
  
  TelnetStream.print("Device_LED : ");
  TelnetStream.println(incomingReadings.device_led);

  TelnetStream.print("Red : ");
  TelnetStream.println(incomingRGB[0]);
  TelnetStream.print("Green : ");
  TelnetStream.println(incomingRGB[1]);
  TelnetStream.print("Blue : ");
  TelnetStream.println(incomingRGB[2]);
  TelnetStream.print("Brightness : ");
  TelnetStream.println(incomingReadings.brightness);
  TelnetStream.print("Sytle : ");
  TelnetStream.println(incomingReadings.style);
  TelnetStream.print("wait Time : ");
  TelnetStream.println(incomingReadings.wait);
  TelnetStream.print("checksum : ");
  TelnetStream.println(incomingReadings.checksum);


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

  
    if(incomingReadings.style == 255){
      server.end();
      Serial.println("Reset WiFi"); 
      TelnetStream.println("Reset WiFi"); 
      changeWiFi();
      server.begin();     
    }
  }

  
  
}

void resetNeopixel(){
  for(int i=0; i < 256; i++){
    pickOneLED(i, strip.Color(0, 0, 0), 0, 0 );
  } 
}




void IRAM_ATTR onTimer0() {
   portENTER_CRITICAL_ISR(&timerMux);
   count0++;
   portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timerMux);
   count1++;
   portEXIT_CRITICAL_ISR(&timerMux);
}


void timer_init(){
   // Configure the Prescaler at 80 the quarter of the ESP32 is cadence at 80Mhz
   // 80000000 / 80 = 1000000 tics / seconds  T: 0.000001s, 1초에 1,000,000 찍음
                 
   timer0 = timerBegin(0, 80, true);
   timer1 = timerBegin(1, 80, true);     
   timerAttachInterrupt(timer0, &onTimer0, true);        
   timerAttachInterrupt(timer1, &onTimer1, true);  
    
   // Sets an alarm to sound every second
   //  number/1,000,000 -> 시간(s)   또는  원하는 시간/0.000001 -> 타이머 인터럽트가 발생되는 Count횟수
   timerAlarmWrite(timer0, 30000000, true);  // 1s
   timerAlarmWrite(timer1, 500000, true); // 15s
   
   timerAlarmEnable(timer0);
//   timerAlarmEnable(timer1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////


void init_SPIFFS(bool format){
   if (!SPIFFS.begin(format)) {
    Serial.println("Failed to mount file system");
    return;
  }
}



void listDir(const char * dirname){
  Serial.printf("Listing directory: %s\r\n", dirname);
  File root = SPIFFS.open(dirname); // ESP8266은 확장자 "Dir"과 "File"로 구분해서 사용, ESP32는 "File"로 통합
  File file = root.openNextFile();
  while(file){ // 다음 파일이 있으면(디렉토리 또는 파일)
    if(file.isDirectory()){ // 다음 파일이 디렉토리 이면
      Serial.print("  DIR : "); Serial.println(file.name()); // 디렉토리 이름 출력
    } else {                // 파일이면
      Serial.print("  FILE: "); Serial.print(file.name());   // 파일이름
      Serial.print("\tSIZE: "); Serial.println(file.size()); // 파일 크기
    }
    file = root.openNextFile();
  }
}

void readFile(const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = SPIFFS.open(path, "r");  // 파을을 열때 읽기 또는 쓰기를 지정하는 옵션이 빠져있다.
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }
  Serial.println("- read from file:");
  while(file.available()){
    Serial.write(file.read());
  }
}

void writeFile(const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = SPIFFS.open(path, "w");   // "w" or FILE_WRITE
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

void appendFile(const char * path, const char * message){
  Serial.printf("Appending to file: %s\r\n", path);
  File file = SPIFFS.open(path, "a");   // "a" or FILE_APPEND
  if(!file){
    Serial.println("- failed to open file for appending");
    return;
  }
  if(file.print(message)){
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
}

void renameFile(const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (SPIFFS.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(const char * path){
  Serial.printf("Deleting file: %s\r\n", path);
  if(SPIFFS.remove(path)){
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

bool saveConfig() { // "SSID":"YOUR_SSID","PASS":"YOUR_PASS","ZONE":9.00,"SUMMER":0,
  String value;
  value = Name("SSID") + strVal(ssid);
  value += Name("PASS") + strVal(pass);
  value += Name("ZONE") + floatNum(timeZone);
  value += Name("SUMMER") + intNum(summerTime);
  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  configFile.println(value); // SPIFF config.txt에 데이터 저장, '\n'포함
  configFile.close();
  return true;
}


bool loadConfig() {
  File configFile = SPIFFS.open("/config.txt", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  String line = configFile.readStringUntil('\n');
  configFile.close();
  String ssidTemp = json_parser(line, "SSID");
  String passTemp = json_parser(line, "PASS");
  stringTo(ssidTemp, passTemp);                // String을 배열에 저장
  String temp = json_parser(line, "ZONE");
  timeZone = temp.toFloat();                   // 스트링을 float로 변환
  temp = json_parser(line, "SUMMER");
  summerTime = temp.toInt();                   // 스트링을 int로 변환
  return true;
}

String json_parser(String s, String a) { 
  String val;
  if (s.indexOf(a) != -1) {
    int st_index = s.indexOf(a);
    int val_index = s.indexOf(':', st_index);
    if (s.charAt(val_index + 1) == '"') {     // 값이 스트링 형식이면
      int ed_index = s.indexOf('"', val_index + 2);
      val = s.substring(val_index + 2, ed_index);
    }
    else {                                   // 값이 스트링 형식이 아니면
      int ed_index = s.indexOf(',', val_index + 1);
      val = s.substring(val_index + 1, ed_index);
    }
  } 
  else {
    Serial.print(a); Serial.println(F(" is not available"));
  }
  return val;
}

String Name(String a) {  
  String temp = "\"{v}\":";
  temp.replace("{v}", a);
  return temp;
}

String strVal(String a) {  
  String temp = "\"{v}\",";
  temp.replace("{v}", a);
  return temp;
}

String intNum(int a) {  
  String temp = "{v},";
  temp.replace("{v}", String(a));
  return temp;
}

String floatNum(float a) {  
  String temp = "{v},";
  temp.replace("{v}", String(a));
  return temp;
}

void stringTo(String ssidTemp, String passTemp) { // 스트링 SSID / PASS 배열에 저장
  for (int i = 0; i < ssidTemp.length(); i++) ssid[i] = ssidTemp[i];
  ssid[ssidTemp.length()] = '\0';
  for (int i = 0; i < passTemp.length(); i++) pass[i] = passTemp[i];
  pass[passTemp.length()] = '\0';
}

void testFileIO(const char * path){
  Serial.printf("Testing file I/O with %s\r\n", path);
  static uint8_t buf[512];
  size_t len = 0;
  File file = SPIFFS.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  size_t i;
  Serial.print("- writing" );
  uint32_t start = millis();
  for(i=0; i<2048; i++){
    if ((i & 0x001F) == 0x001F){
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
  file.close();
  file = SPIFFS.open(path);
  start = millis();
  end = start;
  i = 0;
  if(file && !file.isDirectory()){
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("- reading" );
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F){
        Serial.print(".");
      }
      len -= toRead;
    }
    Serial.println("");
    end = millis() - start;
    Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("- failed to open file for reading");
  }
}


///////////////////////////////////////////////////////////////////////////////////

void initOLED(){
  u8g2.begin();
  
}

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void showOLED_IP_Address(){
  u8g2.clearBuffer();
  u8g2_prepare();
  u8g2.setCursor(0, 15);
  u8g2.print(WiFi.localIP());
  u8g2.sendBuffer();
}


void showOLED_WiFi(char* ssid, char* pass){
  u8g2.clearBuffer();
  u8g2_prepare();
  
  u8g2.setCursor(0, 30);
  u8g2.print(ssid);
  u8g2.setCursor(0, 40);
  u8g2.print(pass);
  
  u8g2.sendBuffer();
  
  

}

void showOLED_changing_WiFi(){
  u8g2.clearBuffer();
  u8g2_prepare();
  
  u8g2.setCursor(0, 15);
  u8g2.print("Changing WiFi");
  u8g2.setCursor(0, 30);
  u8g2.print("WiFi Manager");
  u8g2.setCursor(0, 40);
  u8g2.print("Go to 192.168.4.1");

  u8g2.sendBuffer();

}

/////////////////////////////////////////////////////////

void initWiFi() {
  init_SPIFFS(FORMAT_SPIFFS_IF_FAILED);
  listDir("/");
  Wire.begin(SDA_PIN, SCL_PIN); // SDA, SCL
  initOLED();
  init_Neopixel(50);
  WiFi.mode(WIFI_AP_STA);
  WiFiManager wm;
  Serial.println("--------Saved Data--------");
  if (SPIFFS.exists("/config.txt")) loadConfig();
  else saveConfig();
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  showOLED_WiFi(ssid, pass);
  //readFile("/config.txt");
  Serial.println("--------------------------");
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  pickOneLED(0, strip.Color(255, 0, 0), 5, 50);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    
    Serial.print('.');
    delay(1000);   
    connecting_interval = millis();
    if(connecting_interval >= connecting_interval)
    {
      Serial.println();
      Serial.println("Not Connected -> Open WiFi Manager");
      
      WiFiManager_flag = 1;
      break;
    }
  }
  if(WiFiManager_flag == 1)
  {
    blinkNeopixel(strip.Color(255, 0, 0), 5, 500);
    pickOneLED(0, strip.Color(255, 0, 0), 5, 50);
    wm.resetSettings();
    bool res;
    sprintf(WiFiManager_Name, "RemoteLED_%d", device_id);
    res = wm.autoConnect(WiFiManager_Name);
    
    if(!res) 
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else 
    {
        blinkNeopixel(strip.Color(0, 255, 0), 5, 400);
        pickOneLED(0, strip.Color(0, 255, 0), 5, 50);
        String SSID_NAME = WiFiManager().getWiFiSSID();
        String PW_NAME = WiFiManager().getWiFiPass();

        TelnetStream.println(WiFiManager_Name);
        
        Serial.print("SSID : ");
        Serial.println(SSID_NAME);
        TelnetStream.print("SSID : ");
        TelnetStream.print(SSID_NAME);
        
        Serial.print("PASSWORD : ");
        Serial.println(PW_NAME);
        TelnetStream.print("PASSWORD : ");
        TelnetStream.print(PW_NAME);
        
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        TelnetStream.println("WiFiManager : WiFi Connected");
        
        SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
        PW_NAME.toCharArray(TEMP_PASS, sizeof(TEMP_PASS));

        for(int i=0; i< sizeof(ssid); i++){
          ssid[i] = TEMP_SSID[i];
          pass[i] = TEMP_PASS[i];
        }
        saveConfig();
        
        ESP.restart();
    }
  }
  else 
  {
    Serial.print("connected ->");
    Serial.println(WiFi.localIP());
    TelnetStream.print("connected ->");
    TelnetStream.println(WiFi.localIP());
//    showOLED_IP_Address();
    u8g2.clearBuffer();
    u8g2_prepare();
    u8g2.setCursor(0, 15);
    u8g2.print(WiFi.localIP());
    u8g2.sendBuffer();
    blinkNeopixel(strip.Color(0, 255, 0), 5, 500);
    pickOneLED(0, strip.Color(0, 255, 0), 1, 50);
    
  }
//  readWiFiEEPROM();
  Serial.println("----------Result----------");
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  showOLED_WiFi(ssid, pass);
//  Serial.println(timeZone);
//  Serial.println(summerTime);
  Serial.println("--------------------------");
}

void changeWiFi(){
//  blinkNeopixel(strip.Color(255, 0, 0), 5, 500);
//  pickOneLED(0, strip.Color(255, 0, 0), 50, 50);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFiManager wm;
  wm.resetSettings();
  showOLED_changing_WiFi();
  bool res;
  res = wm.autoConnect("Change_ESP_WiFi");
  if(!res) 
  {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else 
  {
      blinkNeopixel(strip.Color(255, 0, 0), 5, 500);
      String SSID_NAME = WiFiManager().getWiFiSSID();
      String PW_NAME = WiFiManager().getWiFiPass();
      TelnetStream.println(WiFiManager_Name);
      
      Serial.print("SSID : ");
      Serial.println(SSID_NAME);
      TelnetStream.print("SSID : ");
      TelnetStream.print(SSID_NAME);
       
      Serial.print("PASSWORD : ");
      Serial.println(PW_NAME);
      TelnetStream.print("PASSWORD : ");
      TelnetStream.print(PW_NAME);
      
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
      TelnetStream.println("WiFiManager : WiFi Connected");
      SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
      PW_NAME.toCharArray(TEMP_PASS, sizeof(TEMP_PASS));

      for(int i=0; i< sizeof(ssid); i++){
        ssid[i] = TEMP_SSID[i];
        pass[i] = TEMP_PASS[i];
      }
      saveConfig();
      showOLED_IP_Address();
      showOLED_WiFi(ssid, pass);
      pickOneLED(0, strip.Color(0, 255, 0), 50, 2000);
      pickOneLED(0, strip.Color(0, 0, 0), 0, 10);
//      ESP.restart();
  }
}

void reconnectWiFi(){
  if(count0 >0 ){
    portENTER_CRITICAL(&timerMux); // 카운트 시작 
    count0--;                       // 트리거 시간 감소
    portEXIT_CRITICAL(&timerMux);  // 카운트 종료
    Serial.println("reconnect working");
    if ((WiFi.status() != WL_CONNECTED)) {
      Serial.println(millis());
      Serial.println("Reconnecting to WiFi...");
      TelnetStream.println("Reconnecting to WiFi...");
      
      WiFi.disconnect();
      WiFi.reconnect();
      connection_flag = 1;
    }
    else if(connection_flag == 1)
    {
      Serial.println("reconnected");
      TelnetStream.println("reconnected");
      blinkNeopixel(strip.Color(0, 0, 255), 4, 500);
      connection_flag = 0;
    }
  }
}

void blinkTimer(uint32_t color){
  if(count1 > 0){
    portENTER_CRITICAL(&timerMux); // 카운트 시작 
    count1--;                       // 트리거 시간 감소
    portEXIT_CRITICAL(&timerMux);  // 카운트 종료
//    Serial.println("Blink Timer");
    if(neopixel_Flag == 1){
      Serial.print("Blink Status : ");
      Serial.println(blinkToggle);
      blinkToggle = !blinkToggle;
      pickOneLED(0, color, 50*blinkToggle, 50);       
    }     
  }  
}
