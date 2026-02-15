#include <Arduino.h>
#include "Display_SSD1306.h"
#include "HanDraw.h"

#define OLED_RESET -1             // for S/W 리셋
Display_SSD1306 Oled(OLED_RESET);

/************************* Global variables *********************************/
unsigned long startTime = 0;
uint32_t loopcnt = 0;
char fpsbuf[128] = "FPS:";
bool invert = true;  // 화면을 역상으로 표시

void setup() {
  Serial.begin(115200);
  delay(50);

  // Oled를 I2C 방식으로 연결하고, 그 주소는 0x3c
  Oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
  
  // Callback 함수를 설정해 주면 필요시 호출 하여 사용함.
  HanDraw.begin(12, 
    // 화면 지우는 콜백함수 
    [](void) {  Oled.clearDisplay(); },
    // 1개 픽셀을 그리는 콜백 함수
    [](int16_t x, int16_t y, uint16_t color) { Oled.drawPixel(x, y, color); },
    // 메모리에서 Display 표시 버퍼까지 보내는 함수
    [](void) { Oled.display(); }
  );

  HanDraw.display(); // 사실상 Oled.display() 와 동일....
  delay(2000);

  HanDraw.clear();
  HanDraw.setFontSize(12);
  // 특수한 문자 몇개는... 아래와 같이 주면 출력된다. (통용되는 코드가 아니라 변칙 코드임)
  // 12px:한글 21℃℉‰μ°   라고 출력
  HanDraw.drawString(1, 0, "12px:한글 21\x10\x0f\x11\x12\x13"); 
  HanDraw.setFontSize(14);
  // 14:아래첨자 A₁₂₃₄   라고 출력
  HanDraw.drawString(1, 13, "14:아래첨자 A\x0b\x0c\x0d\x0e");
  HanDraw.setFontSize(16);
  // 16:위첨자 M¹²³⁴   라고 출력
  HanDraw.drawString(1, 28, "16:위첨자 M\x15\x16\x17\x18");
  HanDraw.display();
  delay(3000);

  HanDraw.setFontSize(12);
  startTime = millis();
  delay(1);
  Serial.println("Setup All done");
}

String getTime(unsigned long  ttime) {
  int sec = ttime / 1000; int min = sec / 60; int hr = min / 60;
  String ts = ""; 
  if (hr < 10) ts += "0";
  ts += hr;   ts += ":";
  if ((min % 60) < 10) ts += "0";
  ts += min % 60;   ts += ":";
  if ((sec % 60) < 10) ts += "0";
  ts += sec % 60;
  return (ts);
}

void loop() {
  unsigned long  ttime = millis();
  dtostrf(loopcnt * 1000.0 / (ttime - startTime), 5, 2,   fpsbuf + 4);

  HanDraw.clear();
  HanDraw.drawString(15, 2, "[[ 화면 정보 ]]");
  HanDraw.drawString(2, 14, "--------------------");
  HanDraw.drawString(2, 24, "한글 출력 테스트");
  HanDraw.drawString(2, 38, fpsbuf);
  HanDraw.drawString(2, 51, getTime(ttime));
  HanDraw.display();

  loopcnt++;
  if (loopcnt % 100 == 0) {
    Oled.invertDisplay(invert);
    invert = !invert;
  }
}
