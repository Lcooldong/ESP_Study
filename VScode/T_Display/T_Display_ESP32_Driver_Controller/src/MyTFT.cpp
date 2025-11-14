#include "MyTFT.h"
#include "MyLittleFS.h"

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if (y + h > tft.height()) h = tft.height() - y;
  if (x + w > tft.width())  w = tft.width()  - x;

  if (w > 0 && h > 0) {
    tft.pushImage(x, y, w, h, bitmap);
  }

  return 1;
}

void tft_init()
{
  tft.begin(); // Initialize TFT
  tft.setRotation(0); // Set rotation
  tft.fillScreen(TFT_BLACK); // Clear screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color and background
  tft.setTextSize(2); // Set text size
  tft.initDMA();

  // 위 tft 랑 순서 지켜야함
  TJpgDec.setJpgScale(1); // image scale 1(original),2,4,8 
  TJpgDec.setSwapBytes(true); // for color565
  TJpgDec.setCallback(tft_output); // 콜백 함수 등록
}

void drawJpg(int16_t x, int16_t y, const char *pFilename)
{
  uint8_t scale = 1;
  
  if (!LittleFS.exists(pFilename)) {
    Serial.printf("[drawJpg] File not found: %s\n", pFilename);
    return;
  }
  uint16_t jpgWidth, jpgHeight;
  TJpgDec.getFsJpgSize(&jpgWidth, &jpgHeight, pFilename, LittleFS);


  if (jpgWidth > tft.width() || jpgHeight > tft.height()) {
    if (jpgWidth > tft.width() * 2 || jpgHeight > tft.height() * 2)
      scale = 2;
    else
      scale = 1;
  }

  // 
  // while ((jpgWidth / scale > tft.width() || jpgHeight / scale > tft.height()) && scale < 8) {
  //   scale *= 2;
  // }

  Serial.printf("%d|%d(MAX%d|%d) -> %d|%d\r\n", jpgWidth, jpgHeight, tft.width(), tft.height(), jpgWidth/scale, jpgHeight/scale);
  TJpgDec.setJpgScale(scale);


  TJpgDec.drawFsJpg(x, y, pFilename, LittleFS);
  Serial.printf("[drawJpg] Rendered %s at (%d, %d), scale=%d\n", pFilename, x, y, scale);
}

void tft_printf(int16_t x, int16_t y, const char *format, ...)
{
  char buf[512];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  tft.setCursor(x, y); // Set cursor position
  tft.printf(buf); // Print message
}

void tft_clear(int16_t TFT_COLOR)
{
  tft.fillScreen(TFT_COLOR);
}

