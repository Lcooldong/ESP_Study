/*------------------------------------------------------------------
   BSD License

   Copyright (c) 2018 terminal0070@gmail.com

   OLED, LCD등에 한글을 출력하기 위한 라이브러리이다.
   ESP8266의 SPIFFS를 꼭 사용하여야만 한다.
   (아래의 링크에 가면 쉽게 데이터 파일을 업로드 할 수 있다)
   https://github.com/esp8266/arduino-esp8266fs-plugin
   (꼭 소스 디렉토리의 data라는 서브폴더에 위치해 있어야 업로드 가능하다)
   유효한 한글은 euc-kr에 정의된 한글만 표현된다(물론 영숫자는 별도..)

   Arduino의 IDE에서 한글을 기록하면 UTF-8 인코딩이 기본이다.
   이것을 unicode로 변환하고, 다시 euc-kr 코드로 매핑하여 사용한다.
   hfontxx.dat 파일은 앞에 128 * xxbytes로 구성된 ascii 폰트와
   (11 ~ 26번 문자는 다르게 매칭됨. 큰 글자는 2개 문자에 매칭됨..)  "℃℉‰μ°"
   (11 12 13 14       15      16     17    18    18    19          21 22 23 24 25   26)
   0x0b c  d  e       0x0f    0x10  0x11  0x12   0x13   0x14       0x15 16 17  18   1a
   (₁  ₂  ₃  ₄    ℉       ℃    ‰    ‰    μ      °        ¹  ²  ³  ⁴     )

   그 뒤로 euc-kr의 한글 코드 0xb0a1 부터 유효한 한글 글자의 폰트가
   2350 * xx * 2bytes 형태로 기록되어 있다.
   ex) hfont14.dat 이라 하면 한글자당 14 * 2 바이트가 되는셈
   예를 들면 한글이 시작되는 위치에는 아래와 같이('가') 기록되어 있다. (1행 = 2bytes)
    0000000000010000
    0111111000010000
    0000000100010000
    0000000100010000
    0000000100010000
    0000000100011100
    0000000100010000
    0000001000010000
    0000001000010000
    0000010000010000
    0001100000010000
    0110000000010000
    0000000000010000
    0000000000000000
    0000000000000000
    0000000000000000

   3개 크기로 한글폰트를 제작 하였다. (12, 14, 16px)
   (12px 미만은 가독이 너무 어렵고 16px을 초과는 데이터 파일 크기가 커서 만들지 않음)

   utf-euc-map.dat 파일은 UTF-8(결국 유니코드와 같지만..) 한글 코드표 순서의 글자를
   euc-kr 코드표로 매칭하는 데이터가 들어가 있다.
   UTF-8코드표의 11172 글자에 해당하는 매핑표.
   즉 이 파일의 첫번째 두번째 바이트를 연결하면 0xb0a1이며 이며 이는 UTF-8의 첫 글자인
   '가'를 euc-kr코드로 표현한 것이다. 즉 UTF-8 한글 코드표 기준으로 몇번째인지 안다면,
   몇번째 * 2bytes의 위치가 euc-kr의 코드인 것이다.
   (왜 이리 매핑파일을 만들었냐 하면... 이 방법밖에 없기 때문이다. 수식으로 매핑 불가.)

   처음에는 조합형 한글 형태로 개발 하였으나, 늘 컴퓨터에서 보던
   익숙한 완성형 글자의 폰트가 너무 그리워서.. 다시 개발했다.
   그 미려함은 포기 할 수 없으니..
  ------------------------------------------------------------------*/

#ifndef handraw_h
#define handraw_h
#include <Arduino.h>
//#include "SPIFFS.h"
#include "FS.h"

#ifndef DEBUG
//#define DEBUG
#endif

// define callback function
typedef void (*CLEAR_FUNC)(void);
typedef void (*DRAW_PIXEL_FUNC)(int16_t x, int16_t y, uint16_t color);
typedef void (*DISPLAY_FUNC)(void);


// 간단 버전 해쉬테이블..
// 폰트의 경우 일부 내용에 대하여 미리 로딩해 둔다...(어차피 화면에 쓰는 글자는 일부일테니까..)
#define BUCKET_SIZE 5     // 이 크기는 화면에 얼마나 많은 글자를 사용하냐에 따라서 다르다.
#define MAX_LIST_SIZE 30  // 이 크기가 적다면... 매번 화면을 그릴때 마다 특정 글자는 다시 읽어야 되며, 화면 갱신 속도가 느려질 것이다.

class FontHashMap {
  public:
    FontHashMap() {};   ~FontHashMap() {};
    void setValue(unsigned int key, unsigned char *value) {
      byte bucket =  getBucketIndex(key);
      // 버킷이 꽉 차있으면 맨처음으로 돌리고.. 그리고 free해서 비운다...
      if (addIndex[bucket] >= MAX_LIST_SIZE)  addIndex[bucket] = 0;
      if (valueArray[bucket][addIndex[bucket]] != NULL) free(valueArray[bucket][addIndex[bucket]]);
      keyArray[bucket][addIndex[bucket]] =  key;
      valueArray[bucket][addIndex[bucket]++] = value;
    }
    unsigned char * getValue(unsigned int key) {
      byte bucket =  getBucketIndex(key);
      for (int i = 0; i < MAX_LIST_SIZE; i ++) {
        if (key == keyArray[bucket][i] )   return valueArray[bucket][i];
      }
      return NULL;
    }
    uint8_t getBucketIndex(unsigned int key) {
      return (key / 100) % BUCKET_SIZE;
    }
  private:
    byte addIndex[BUCKET_SIZE];
    unsigned int   keyArray[BUCKET_SIZE][MAX_LIST_SIZE];
    unsigned char *valueArray[BUCKET_SIZE][MAX_LIST_SIZE];
};

class HanDrawClass  {
  protected:
    int8_t m_dCharSize = 16;  // 한글 기준 픽셀 크기
    CLEAR_FUNC      m_callbackClearFunction;     // 화면 지우기용 콜백
    DRAW_PIXEL_FUNC m_callbackDrawPixelFunction; // 1개 점찍기용 콜백
    DISPLAY_FUNC    m_callbackDisplayFunction;   // 버퍼 -> Display용 콜백

    // 아래 두개 숫자는 오래 걸리는 것은 아니나 항상 쓰는 숫자 이기 때문에. 미리 계산 해 둠
    int8_t m_dCharHalfSize = 8; // 영문 기준 픽셀 크기 (자동 계산됨)
    int8_t m_dCharSecondSize = 8; // 한글기준 두번째 바이트 그리는 범위 (자동 계산됨) 한글자 크기가 14면 이값은 14 - 8 = 6
    File m_fpFontData;               // 한글 폰트 정보가 있는 파일
    File m_fpUtf8_Euckr_mapper;      // UTF8을 Euckr로 바꾸기 위한 매핑 테이블 파일
    FontHashMap m_HanFontHashMap[3]; // 한글폰트 캐시를 위하여 읽은 문자를 저장해 두는곳...
    FontHashMap m_EngFontHashMap[3]; // 영어폰트 캐시를 위하여 읽은 문자를 저장해 두는곳...

  private:
    void unicode2Euckr(unsigned int unicode, int8_t *out1, int8_t *out2);
    void drawOneHan(int16_t x, int16_t y, int8_t byte1, int8_t byte2, int8_t byte3);
    void drawOneSpecial(int16_t x, int16_t y, int8_t byte1, int8_t byte2, int8_t byte3);
    void drawOneEng(int16_t x, int16_t y, int16_t start, int16_t end, int8_t ascii);

  public:
    HanDrawClass();
    virtual ~HanDrawClass();
    int8_t begin(int8_t fontSize, CLEAR_FUNC clearFunction , DRAW_PIXEL_FUNC drawPixelFunction, DISPLAY_FUNC displayFunction );
    void setFontSize(int8_t fontSize);
    int8_t end();
    void clear();
    void display();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawString(int16_t x, int16_t y, char* message);
    void drawString(int16_t x, int16_t y, const char* message);
    void drawString(int16_t x, int16_t y, String message);
};
extern HanDrawClass HanDraw;
#endif
