#include <Arduino.h>
#include <LittleFS.h>
#include <HardwareSerial.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <core_version.h> // For ARDUINO_ESP32_RELEASE

#include <TFT_eSPI.h>
#include <OneButton.h>
#include <ACAN_ESP32.h>
#include <RotaryEncoder.h> // Include the RotaryEncoder library
#include <PNGdec.h>

#define BUTTON_PIN     GPIO_NUM_35 // GPIO pin for the button

#define ROTARY_CLK_PIN GPIO_NUM_32
#define ROTARY_DT_PIN  GPIO_NUM_33
#define ROTARY_SW_PIN  GPIO_NUM_37 // 34~39 pull-up impossible

#define CAN_RX_PIN     GPIO_NUM_25 // CAN RX pin
#define CAN_TX_PIN     GPIO_NUM_26 // CAN TX pin

#define LED_PIN        GPIO_NUM_17 // GPIO pin for the LED (not used in this example)
#define LED_CHANNEL    0 // LED channel for PWM (not used in this example)
#define LED_FREQUENCY  5000 // Frequency for PWM (not used in this example)
#define LED_RESOLUTION 8 // Resolution for PWM (not used in this example)

PNG png;
File pngfile;
#define MAX_IMAGE_WIDTH 240 // Adjust for your images

int16_t xpos = 0;
int16_t ypos = 0;

const int NUM_ITEM = 4; // Number of menu items
const int MAX_ITEM_LENGTH = 20; // Maximum length of each menu item
const uint32_t DESIRED_BIT_RATE = 500UL * 1000UL ; // 500 kb/s

enum {
  First_MENU = 0,
  Second_MENU = 1,
  Third_MENU = 2,
  Fourth_MENU = 3,
};

char menu_items[NUM_ITEM][MAX_ITEM_LENGTH] = {{"FIRST MENU"}, {"SECOND MENU"}, {"THIRD MENU"}, {"FOURTH MENU"}};
int item_selected = 0; // Current menu index
int current_screen = 0;
int item_sel_previous;
int item_sel_next;


uint32_t currentMillis = 0;
uint32_t previousMillis[4] = {0,};
int counter = 0;
uint8_t ledValue = 0;
bool can_switch = false;

TFT_eSPI tft = TFT_eSPI(); // Invoke library
OneButton button; // Button on GPIO 0, pull-up enabled
OneButton rotary_sw;

HardwareSerial Serail1(1);  // UART1
RotaryEncoder rotaryEncoder(ROTARY_CLK_PIN, ROTARY_DT_PIN, RotaryEncoder::LatchMode::TWO03); // Rotary encoder object
ACAN_ESP32 & my_can = ACAN_ESP32::can; // Create a reference to the CAN object

int renderWidth = 240;     // 원하는 출력 크기
int renderHeight = 135;
float scaleX = 1.0f;
float scaleY = 1.0f;
uint8_t gammaTable[256];

void button_click();
void rotary_sw_click();
void encoder_update();
void serial_command();
bool can_init();
bool can_send(int id, uint8_t *data, size_t len);
CANMessage can_receive();
bool can_connect();

void * pngOpen(const char *filename, int32_t *size);
void pngClose(void *handle);
int32_t pngRead(PNGFILE *handle, uint8_t *buffer, int32_t length);
int32_t pngSeek(PNGFILE *handle, int32_t position);
void pngDraw(PNGDRAW *pDraw);
void pngDraw2(PNGDRAW *pDraw);
// void drawPngFromLittleFS(const char *path, int x, int y);
void drawPngFromLittleFS(const char *path, int x, int y, int targetW, int targetH);
void drawPngScaled(const char* path, int dstW, int dstH, int posX, int posY);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void initGammaTable(float gamma);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, GPIO_NUM_13, GPIO_NUM_15); // UART1 with RX on GPIO 16 and TX on GPIO 17
  ledcAttachPin(LED_PIN, LED_CHANNEL); // Attach LED pin to channel (not used in this example)
  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION); // Setup LED channel (not used in this example)
  ledcWrite(LED_CHANNEL, 0); // Set LED brightness to 50% (not used in this example)

  // Button and rotary switch setup
  button.setup(BUTTON_PIN, INPUT, true); // Circuit pull-up
  button.attachClick(button_click);
  rotary_sw.setup(ROTARY_SW_PIN, INPUT, true); // Setup rotary switch with pull-up
  rotary_sw.attachClick(rotary_sw_click);

  can_init();
  delay(100); 

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  } else {
    Serial.println("LittleFS mounted!");
  }

  Serial.printf("FLASH: %d MB\r\n", ESP.getFlashChipSize()/(1024*1024)); // Should print 16777216 for 16MB

  tft.begin(); // Initialize TFT
  tft.setRotation(1); // Set rotation
  tft.fillScreen(TFT_BLACK); // Clear screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color and background
  tft.setTextSize(2); // Set text size
  tft.initDMA(); 
  initGammaTable(2.2f);


}

void loop() {
  currentMillis = millis();

  if (currentMillis - previousMillis[0] >= 1000) { // Update every second
    previousMillis[0] = currentMillis;
    counter++;
    Serial.printf("Counter: %d %d|\r\n", counter, digitalRead(ROTARY_SW_PIN)); // Print counter and button state to serial
    Serial1.printf("Serial1: %d\r\n", counter); // Send counter value over UART1

    // tft.fillScreen(TFT_BLACK); // Clear screen
    
    tft.setCursor(10, 5); // Set cursor position
    tft.printf("[%d]", counter); // Print message

    if(can_switch)
    {
      uint8_t test[8] = {0,};
      for (int i = 0; i < 8; i++)
      {
        test[i] = i + counter; // Fill test array with some data
      }
      can_send(0x123, test, sizeof(test)/sizeof(test[0]));
    }
  }
  else if (currentMillis - previousMillis[1] >= 10) { // Update every 500ms
    previousMillis[1] = currentMillis;
    button.tick(); // Check button state
    rotary_sw.tick(); // Check rotary switch state
  }
  else if (currentMillis - previousMillis[2] >= 3) { // Update every 5ms
    previousMillis[2] = currentMillis;
    encoder_update();
     
  }
  else
  {
    can_receive(); // Check for received CAN messages
    serial_command(); // Handle serial commands
  }
}



void button_click() {
  Serial.println("Button clicked!");
  // tft.fillScreen(TFT_RED); // Change screen color on button click
}
void rotary_sw_click() {
  Serial.println("Rotary switch clicked!");
  // tft.fillScreen(TFT_BLUE); // Change screen color on rotary switch click
}

void encoder_update()
{
  static int pos = 0;

  rotaryEncoder.tick();
  
  int newPos = (int)(rotaryEncoder.getPosition()/2); // Two steps per click
  if (pos != newPos) {          // changed in pos
    Serial.print("pos:");
    Serial.print(newPos);
    int dir = (int)(rotaryEncoder.getDirection());
    Serial.print(" dir:");
    Serial.println(dir);

    item_sel_previous = item_selected; // Store previous item index
    if(dir == 1){
      item_selected++;
      if(item_selected >= sizeof(menu_items)/sizeof(menu_items[0])) {
        item_selected = 0; // Wrap around to the first menu
      }
    }
    else if(dir == -1){
      item_selected--;
      if(item_selected < 0) {
        item_selected = sizeof(menu_items)/sizeof(menu_items[0]) - 1; // Wrap around to the last menu
      }
    }
    pos = newPos;
    Serial.printf("Item selected: %s\r\n", menu_items[item_selected]); // Print selected item index to serial
  }
}

void serial_command()
{
  if(Serial.available()) {
    char c = Serial.read();
    switch (c)
    {
    case '1': 
      ledValue++;
      break;
    case '2': 
      ledValue--;
      break;
    case '3':
      can_switch = !can_switch;
      break;
    case '4':
      Serial.printf("-------------------------Restart-----------------------\r\n");
      ESP.restart();
      break;
    case '5':
      // drawPngFromLittleFS("/panda135_180.png", 20, 20);
      drawPngFromLittleFS("/panda.png", 20, 25, 100, 200);
      break;
    case '6':
      drawPngScaled("/panda.png", 240, 135, 0, 0);
      break;

    case '0':
      listDir(LittleFS, "/", 0);
      break;
    }
    ledcWrite(LED_CHANNEL, ledValue); // Set LED brightness to 50% (not used in this example)
    Serial.printf("LED Value: %d\r\n", ledValue); // Print LED value to serial
  }
}

bool can_init()
{
 bool ret = false;

  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE) ;
  settings.mRxPin = CAN_RX_PIN ; 
  settings.mTxPin = CAN_TX_PIN ;
  settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode ;
  const uint32_t errorCode = my_can.begin (settings) ;

  if (errorCode == 0) {
    Serial.printf("APB Frequency     : %d MHz\r\n", getApbFrequency()/1000000);
    Serial.print ("Bit Rate prescaler: ") ;
    Serial.println (settings.mBitRatePrescaler) ;
    Serial.print ("Time Segment 1:     ") ;
    Serial.println (settings.mTimeSegment1) ;
    Serial.print ("Time Segment 2:     ") ;
    Serial.println (settings.mTimeSegment2) ;
    Serial.print ("RJW:                ") ;
    Serial.println (settings.mRJW) ;
    Serial.print ("Triple Sampling:    ") ;
    Serial.println (settings.mTripleSampling ? "yes" : "no") ;
    Serial.print ("Actual bit rate:    ") ;
    Serial.print (settings.actualBitRate ()) ;
    Serial.println (" bit/s") ;
    Serial.print ("Exact bit rate ?    ") ;
    Serial.println (settings.exactBitRate () ? "yes" : "no") ;
    Serial.print ("Distance            ") ;
    Serial.print (settings.ppmFromDesiredBitRate ()) ;
    Serial.println (" ppm") ;
    Serial.print ("Sample point:       ") ;
    Serial.print (settings.samplePointFromBitStart ()) ;
    Serial.println ("%") ;
    Serial.println ("Configuration OK!");

    ret = true;
  }else {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

 return ret;

}

bool can_send(int id, uint8_t *data, size_t len)
{
  static uint32_t sent_frame_count = 0; // Static variable to keep track of sent frames
  CANMessage frame ;
  
  frame.id = id ;
  frame.len = len ;
  if(data != NULL && len > 0) {
    memcpy(frame.data, data, len); // Copy data to frame
  }
  
  const bool ok = my_can.tryToSend (frame) ;
  if (ok) {
    sent_frame_count++;
    Serial.printf("[%d]CAN frame sent: ID=0x%X, Data=", sent_frame_count, frame.id);
    for (size_t i = 0; i < len; i++) {
      Serial.printf("%02X ", frame.data[i]);
    }
    Serial.println();
    
  } else {
    Serial.printf("Failed to send CAN frame\r\n");
    if(my_can.driverTransmitBufferSize() < my_can.driverTransmitBufferPeakCount())
    {
      Serial.printf("ESP Restart for can reconnect\r\n");
      ESP.restart();
    }
  }


  return ok; // Return true if the frame was sent successfully
}




CANMessage can_receive()
{
  
  static uint32_t received_frame_count = 0; // Static variable to keep track of received frames
  CANMessage frame;

  if(my_can.available()) {
    my_can.receive(frame);
    
    received_frame_count++;
    Serial.printf("[%d]CAN frame received: ID=0x%X, Data=", received_frame_count, frame.id);
    for (size_t i = 0; i < frame.len; i++) {
      Serial.printf("%02X ", frame.data[i]);
    }
    Serial.println();
    
  } 
  return frame; // Return the received frame
}

// only for receive can't reconnect
bool can_connect()
{
  bool ret = false;
  my_can.kNotInResetModeInConfiguration;
  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE);
  settings.mRxPin = CAN_RX_PIN ; 
  settings.mTxPin = CAN_TX_PIN ;
  settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode ;

  const uint32_t errorCode = my_can.begin (settings) ;
  // const uint32_t errorCode = ACAN_ESP32::can.begin (settings) ;
  delay(50);
  if(errorCode == 0) {
    Serial.println("CAN connection established successfully!");
    ret = true;
  } else {
    Serial.printf("CAN connection failed with error code: 0x%X\r\n", errorCode);
  }
  

  return  ret;
}

void *pngOpen(const char *filename, int32_t *size) {
  pngfile = LittleFS.open(filename, "r");
  if (!pngfile) {
    Serial.printf("Failed to open file: %s\n", filename);
    *size = 0;
    return nullptr;
  }
  *size = pngfile.size();
  return (void *)&pngfile;
}

// ⬛ 파일 닫기 콜백
void pngClose(void *handle) {
  File *f = (File *)handle;
  if (f && *f) f->close();
}

// ⬛ 읽기 콜백
int32_t pngRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  File *f = (File *)handle->fHandle;
  return f->read(buffer, length);
}

// ⬛ 시크 콜백
int32_t pngSeek(PNGFILE *handle, int32_t position) {
  File *f = (File *)handle->fHandle;
  return f->seek(position);
}

uint8_t applyGamma(uint8_t value, float gamma = 2.2) {
  return powf(value / 255.0f, gamma) * 255;
}

// void pngDraw(PNGDRAW *pDraw) {
//   static uint16_t lineBuffer[MAX_IMAGE_WIDTH];

//   for (int i = 0; i < pDraw->iWidth; i++) {
//     // 원본 RGB 값
//     uint8_t r = pDraw->pPixels[i * 3 + 0];
//     uint8_t g = pDraw->pPixels[i * 3 + 1];
//     uint8_t b = pDraw->pPixels[i * 3 + 2];

//     // 감마 보정
//     r = applyGamma(r);
//     g = applyGamma(g);
//     b = applyGamma(b);

//     // 채도/명도 boost (선택)
//     r = min((int)(r * 1.2), 255);
//     g = min((int)(g * 1.2), 255);
//     b = min((int)(b * 1.2), 255);

//     lineBuffer[i] = tft.color565(r, g, b);
//   }

//   tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
//   Serial.printf("Line: %d\r\n", pDraw->y);
// }



void initGammaTable(float gamma) {
  for (int i = 0; i < 256; i++) {
    gammaTable[i] = (uint8_t)(pow((float)i / 255.0f, gamma) * 255.0f + 0.5f);
  }
}

void pngDraw(PNGDRAW *pDraw) {
  static uint16_t lineBuffer[135]; // 최대 폭에 맞춰

  int scaled_y = pDraw->y / scaleY;
  if (scaled_y * scaleY != pDraw->y) return; // 축소 시 생략

  for (int x = 0; x < renderWidth; x++) {
    int srcX = x * scaleX;
    uint8_t r = gammaTable[pDraw->pPixels[srcX * 3 + 0]];
    uint8_t g = gammaTable[pDraw->pPixels[srcX * 3 + 1]];
    uint8_t b = gammaTable[pDraw->pPixels[srcX * 3 + 2]];
    lineBuffer[x] = tft.color565(r, g, b);
  }

  tft.pushImage(xpos, ypos + scaled_y, renderWidth, 1, lineBuffer);
  Serial.printf("Line: %d\r\n", pDraw->y);
}

// // ⬛ 이미지 하나 로드 및 디코드
// void drawPngFromLittleFS(const char *path, int x, int y) {
//   xpos = x;
//   ypos = y;

//   int32_t rc = png.open(path, pngOpen, pngClose, pngRead, pngSeek, (PNG_DRAW_CALLBACK *)pngDraw);
//   if (rc == PNG_SUCCESS) {
//     Serial.printf("PNG opened: %s (%d x %d)\n", path, png.getWidth(), png.getHeight());
//     png.decode(NULL, 0);
//     png.close();
//   } else {
//     Serial.printf("PNG open failed: %s (rc = %d)\n", path, rc);
//     // switch (rc) {
//     //   case PNG_INVALID_FILE_HEADER: Serial.println(" → Invalid file header"); break;
//     //   case PNG_UNSUPPORTED_FEATURE: Serial.println(" → Unsupported PNG feature"); break;
//     //   case PNG_OUT_OF_MEMORY:       Serial.println(" → Out of memory"); break;
//     //   case PNG_FILE_ERROR:          Serial.println(" → File access error"); break;
//     //   default:                      Serial.println(" → Unknown error"); break;
//     // }
//   }
// }

void drawPngFromLittleFS(const char *path, int x, int y, int targetW, int targetH) {
  xpos = x;
  ypos = y;

  int32_t rc = png.open(path, pngOpen, pngClose, pngRead, pngSeek, (PNG_DRAW_CALLBACK *)pngDraw);
  if (rc == PNG_SUCCESS) {
    int origW = png.getWidth();
    int origH = png.getHeight();

    renderWidth = targetW;
    renderHeight = targetH;
    scaleX = (float)origW / targetW;
    scaleY = (float)origH / targetH;

    Serial.printf("PNG [%s]: %dx%d → %dx%d\n", path, origW, origH, targetW, targetH);

    png.decode(NULL, 0);
    png.close();
  } else {
    Serial.printf("PNG open failed: %s (rc = %d)\n", path, rc);
  }
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{

    Serial.println("-------------------------------------");
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }
    
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
            Serial.println(file.name());
#else
            Serial.print(file.name());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
#endif

            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            // USBSFerial.print(file.name());
            Serial.print("  SIZE: ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
            Serial.println(file.size());
#else
            Serial.print(file.size());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
#endif
        }
        file = root.openNextFile();
    }
    Serial.println("-------------------------------------");
}

int pngSrcWidth = 0, pngSrcHeight = 0;
int scaledWidth = 0, scaledHeight = 0;
float scaleFactor = 1.0;
int drawX = 0, drawY = 0;


int pngDstWidth = 135, pngDstHeight = 240;

void pngDraw2(PNGDRAW *pDraw) {
  static uint16_t lineBuf[240];

  int yOut = (int)(pDraw->y * scaleFactor + 0.5); // 반올림
  if (yOut >= scaledHeight) return;

  for (int xOut = 0; xOut < scaledWidth; xOut++) {
    int xIn = (int)(xOut / scaleFactor + 0.5); // 반올림
    if (xIn >= pDraw->iWidth) continue;

    int idx = xIn * 3;
    if (idx + 2 >= pDraw->iWidth * 3) continue;

    uint8_t r = gammaTable[pDraw->pPixels[idx]];
    uint8_t g = gammaTable[pDraw->pPixels[idx + 1]];
    uint8_t b = gammaTable[pDraw->pPixels[idx + 2]];

    // 채도 향상
    r = min((int)(r * 1.2), 255);
    g = min((int)(g * 1.2), 255);
    b = min((int)(b * 1.2), 255);

    lineBuf[xOut] = tft.color565(r, g, b);
  }

  tft.pushImage(drawX, drawY + yOut, scaledWidth, 1, lineBuf);
  Serial.printf("Line: %d\r\n", yOut);
}

void drawPngScaled(const char* path, int dstW, int dstH, int posX, int posY) {
  drawX = posX;
  drawY = posY;
  pngDstWidth = dstW;
  pngDstHeight = dstH;

  int rc = png.open(path, pngOpen, pngClose, pngRead, pngSeek, (PNG_DRAW_CALLBACK *)pngDraw2);
  if (rc != PNG_SUCCESS) {
    Serial.printf("PNG open failed (%d): %s\n", rc, path);
    return;
  }

  pngSrcWidth = png.getWidth();
  pngSrcHeight = png.getHeight();

  Serial.printf("Image: %dx%d → %dx%d at (%d,%d)\n",
    pngSrcWidth, pngSrcHeight, pngDstWidth, pngDstHeight, drawX, drawY);

  png.decode(NULL, 0);
  png.close();
}
const int MAX_WIDTH = 135;






// void pngDraw(PNGDRAW *pDraw) {
//   static uint16_t lineBuffer[MAX_IMAGE_WIDTH];

//   for (int i = 0; i < pDraw->iWidth; i++) {
//     // 원본 RGB 값
//     uint8_t r = pDraw->pPixels[i * 3 + 0];
//     uint8_t g = pDraw->pPixels[i * 3 + 1];
//     uint8_t b = pDraw->pPixels[i * 3 + 2];

//     // 감마 보정
//     r = applyGamma(r);
//     g = applyGamma(g);
//     b = applyGamma(b);

//     // 채도/명도 boost (선택)
//     r = min((int)(r * 1.2), 255);
//     g = min((int)(g * 1.2), 255);
//     b = min((int)(b * 1.2), 255);

//     lineBuffer[i] = tft.color565(r, g, b);
//   }

//   tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
//   Serial.printf("Line: %d\r\n", pDraw->y);
// }


// void pngDraw2(PNGDRAW *pDraw) {
//   // 비율 계산
//   float scaleX = (float)pngSrcWidth / pngDstWidth;
//   float scaleY = (float)pngSrcHeight / pngDstHeight;

//   // 현재 라인이 출력 대상인지 확인
//   int yOut = (int)(pDraw->y / scaleY);
//   if (yOut >= pngDstHeight) return;

//   // 출력용 버퍼
//   static uint16_t lineBuf[135]; // 최대 크기

//   for (int xOut = 0; xOut < pngDstWidth; xOut++) {
//     int xIn = (int)(xOut * scaleX);
//     int idx = xIn * 3;
//     if (idx + 2 >= pDraw->iWidth * 3) continue;

//     uint8_t r = gammaTable[pDraw->pPixels[idx + 0]];
//     uint8_t g = gammaTable[pDraw->pPixels[idx + 1]];
//     uint8_t b = gammaTable[pDraw->pPixels[idx + 2]];
//     lineBuf[xOut] = tft.color565(r, g, b);
//   }

//   tft.pushImage(drawX, drawY + yOut, pngDstWidth, 1, lineBuf);
//   Serial.printf("Line: %d\r\n", pDraw->y);
// }