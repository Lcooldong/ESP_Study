/*
   BSD License
  
   Copyright (c) 2018 terminal0070@gmail.com
*/

#include "HanDraw.h"

#ifndef BLACK
#define BLACK 0
#endif

#ifndef WHITE
#define WHITE 1
#endif

HanDrawClass::HanDrawClass() {}
HanDrawClass::~HanDrawClass() { end();}

// 시작시 몇 사이즈의 폰트 크기를 사용할 것인가를 정한다.
// 3개의 callback function을 이용하여 디스플레이를 제어한다.
int8_t HanDrawClass::begin(int8_t fontSize, CLEAR_FUNC clearFunction, DRAW_PIXEL_FUNC drawPixelFunction, DISPLAY_FUNC displayFunction) {
  m_callbackClearFunction = clearFunction;
  m_callbackDrawPixelFunction = drawPixelFunction;
  m_callbackDisplayFunction = displayFunction;
  bool result = SPIFFS.begin();
  //bool result = FS.begin();
#ifdef DEBUG
  Serial.println("SPIFFS opened: " + result);
#endif
  if (result) {
    setFontSize(fontSize);
    m_fpUtf8_Euckr_mapper = SPIFFS.open("/utf-euc-map.dat", "r");
    //m_fpUtf8_Euckr_mapper = FS.open("/utf-euc-map.dat", "r");
#ifdef DEBUG
    Serial.print("Font file handle: ");
    Serial.println(m_fpFontData);
    Serial.print("map file handle: ");
    Serial.println(m_fpUtf8_Euckr_mapper);
#endif
  }
  return result;
}

// 폰트 크기를 정한다. 
void HanDrawClass::setFontSize(int8_t fontSize) {
  m_dCharSize = fontSize;
  m_dCharHalfSize = (int8_t)(fontSize / 2);
  m_dCharSecondSize = (int8_t)(fontSize - 8);
  if (m_fpFontData != 0x00)
    m_fpFontData.close();
  char fontFileName[64];
  sprintf(fontFileName, "/hfont%d.dat", m_dCharSize);
  m_fpFontData = SPIFFS.open(fontFileName, "r");
  //m_fpFontData = FS.open(fontFileName, "r");
}

// 종료되는 시점에는 파일을 닫아 버린다.
int8_t HanDrawClass::end() {
  m_fpFontData.close();
  m_fpUtf8_Euckr_mapper.close();
  return 1;
}

// 화면을 지우는 것은 콜백 함수를 부른다.
void HanDrawClass::clear() {
  m_callbackClearFunction();
}

// 역시 화면에 표시하는 것은 콜백 함수를 부른다
void HanDrawClass::display() {
  m_callbackDisplayFunction();
}

void HanDrawClass::drawPixel(int16_t x, int16_t y, uint16_t color) {
#ifdef DEBUG
  Serial.print(color);
#endif
  m_callbackDrawPixelFunction(x, y, color);
}

// 아래 특수 문자는 좀더 특별한 방법으로 .. 표시한다.
//  0x0f    0x10  0x11  0x12   0x13   0x14
// ℉       ℃    ‰    ‰    μ      °
void HanDrawClass::drawOneSpecial(int16_t x, int16_t y, int8_t byte1, int8_t byte2, int8_t byte3) {
  unsigned int unicode = (byte1 & 0b00001111) << 12 | (byte2 & 0b00111111) << 6 | (byte3 & 0b00111111);
  unsigned char *ch;
  int hashIndex = (m_dCharSize - 12) / 2;  //지원하는 폰트 사이즈가 12, 14,16 이며.. 해쉬테이블의 0,1,2 배열에 해당
  // 문자 크기를 못 찾으면 ... 무시..
  if (hashIndex < 0 || hashIndex > 2 )
    return;

  ch = m_HanFontHashMap[hashIndex].getValue(unicode);
  //못 찾았으면..... 읽어서 해쉬 테이블에 세팅하고..
  if (NULL == ch) {
    int16_t charIndex = 2350 + (byte3 - 0x0f);
    ch = (unsigned char *)malloc(m_dCharSize * 2);
    m_fpFontData.seek(charIndex * m_dCharSize * 2 + 128 * m_dCharSize, SeekSet); // 앞에 영문자 뺴고.. (스킵해야지...)
    m_fpFontData.read(ch, m_dCharSize * 2);
    m_HanFontHashMap[hashIndex].setValue(unicode, ch);
  }

  uint8_t dataIdx = 0;
  for ( int16_t i = 0; i < m_dCharSize; i++) {
    for ( int16_t k = 0; k < 8; k++) {
      if ( ( ( ch[dataIdx] >> (7 - k) ) & 0b00000001 ) != 0 )
        drawPixel(x + k, y + i, WHITE);
#ifdef DEBUG
      else
        drawPixel(x + k, y + i, BLACK);
#endif
    }
    dataIdx++;

    for ( int16_t k = 0; k < m_dCharSecondSize; k++)  {
      if ( ( ( ch[dataIdx] >> (7 - k) ) & 0b00000001 ) != 0 )
        drawPixel(x + k + 8, y + i, WHITE);
#ifdef DEBUG
      else
        drawPixel(x + k + 8, y + i, BLACK);
#endif
    }
    dataIdx++;

#ifdef DEBUG
    Serial.println("");
#endif
  }
#ifdef DEBUG
  Serial.println("");
#endif

}

// x, y, euc-kr 상위 바이트,  하위 바이트 순서
void HanDrawClass::drawOneHan(int16_t x, int16_t y, int8_t byte1, int8_t byte2, int8_t byte3) { //, int8_t upbyte, int8_t downbyte, unsigned int unicode) {
  // utf-8의 3byte byte1, byte2, byte3의 1110xxxx, 10xxxxxx, 10xxxxxx의 부분만 가져오면 유니코드
  unsigned int unicode = (byte1 & 0b00001111) << 12 | (byte2 & 0b00111111) << 6 | (byte3 & 0b00111111);
  unsigned char *ch;
  int hashIndex = (m_dCharSize - 12) / 2;  //지원하는 폰트 사이즈가 12, 14,16 이며.. 해쉬테이블의 0,1,2 배열에 해당
  // 문자 크기를 못 찾으면 ... 무시..
  if (hashIndex < 0 || hashIndex > 2 )
    return;

  // 해쉬 테이블에서 먼저 찾고
  ch = m_HanFontHashMap[hashIndex].getValue(unicode);
  //못 찾았으면..... 읽어서 해쉬 테이블에 세팅하고..
  if (NULL == ch) {
    int8_t upbyte, downbyte; // euc-kr  first, second byte
    unicode2Euckr(unicode, &upbyte, &downbyte);
    int16_t charIndex = (((uint8_t)upbyte - 0xb0)  * 94) + ((uint8_t)downbyte - 0xa1);
    ch = (unsigned char *)malloc(m_dCharSize * 2);
    m_fpFontData.seek(charIndex * m_dCharSize * 2 + 128 * m_dCharSize, SeekSet); // 앞에 영문자 뺴고.. (스킵해야지...)
    m_fpFontData.read(ch, m_dCharSize * 2);
    m_HanFontHashMap[hashIndex].setValue(unicode, ch);
  }


  //출력 못하는 글자는 대충 짝대기를 쭈욱 그어 준다..
  //  if ( (uint8_t)upbyte < 0xb0 || (uint8_t)downbyte < 0xa0 ) {
  //    drawOneEng(x, y, 0, m_dCharHalfSize,  0x2d);
  //    drawOneEng(x + (int16_t)(m_dCharHalfSize), y, 0, m_dCharHalfSize, 0x2d);
  //    return;
  //  }

  uint8_t dataIdx = 0;
  for ( int16_t i = 0; i < m_dCharSize; i++) {
    for ( int16_t k = 0; k < 8; k++) {
      if ( ( ( ch[dataIdx] >> (7 - k) ) & 0b00000001 ) != 0 )
        drawPixel(x + k, y + i, WHITE);
#ifdef DEBUG
      else
        drawPixel(x + k, y + i, BLACK);
#endif
    }
    dataIdx++;

    for ( int16_t k = 0; k < m_dCharSecondSize; k++)  {
      if ( ( ( ch[dataIdx] >> (7 - k) ) & 0b00000001 ) != 0 )
        drawPixel(x + k + 8, y + i, WHITE);
#ifdef DEBUG
      else
        drawPixel(x + k + 8, y + i, BLACK);
#endif
    }
    dataIdx++;

#ifdef DEBUG
    Serial.println("");
#endif
  }
#ifdef DEBUG
  Serial.println("");
#endif
}

void HanDrawClass::drawOneEng(int16_t x, int16_t y, int16_t start, int16_t end, int8_t ascii) {
  if (ascii == 0x20)
    return; // 스페이스 문자임...

  unsigned char *ch;//[m_dCharSize * 2];
  int hashIndex = (m_dCharSize - 12) / 2;  //지원하는 폰트 사이즈가 12, 14,16 이며.. 해쉬테이블의 0,1,2 배열에 해당

  // 문자 크기를 못 찾으면 ... 무시..
  if (hashIndex < 0 || hashIndex > 2 )
    return;
  // 해쉬 테이블에서 먼저 찾고
  ch = m_EngFontHashMap[hashIndex].getValue(ascii * 100);
  //못 찾았으면..... 읽어서 해쉬 테이블에 세팅하고..
  if (NULL == ch) {
    ch = (unsigned char *)malloc(m_dCharSize);
    m_fpFontData.seek(ascii * m_dCharSize, SeekSet);
    m_fpFontData.read(ch, m_dCharSize);
    m_EngFontHashMap[hashIndex].setValue(ascii * 100, ch);
  }


  uint8_t dataIdx = 0;
  for ( int16_t i = 0; i < m_dCharSize; i++) {
    //상대적으로 너비가 좁은  i, l, 1, I 등은 좁게 그림...
    for ( int16_t k = start; k < end; k++) {
      if ( ( ( ch[dataIdx] >> (7 - k) ) & 0b00000001 ) != 0 )
        drawPixel(x + k, y + i, WHITE);
#ifdef DEBUG
      else
        drawPixel(x + k, y + i, BLACK);
#endif
    }
    dataIdx++;
#ifdef DEBUG
    Serial.println("");
#endif
  }
#ifdef DEBUG
  Serial.println("");
#endif
}

// 유니코드를 주면 2바이트의 euc-kr을 계산 해주는...함수
void HanDrawClass::unicode2Euckr(unsigned int unicode, int8_t *out1, int8_t *out2) {
  // 여기에서  0xac00을 빼주면 0 부터의 인덱스가 된다.
  m_fpUtf8_Euckr_mapper.seek((unicode - 0xac00) * 2, SeekSet); // 2바이트씩 들어감
  *out1 = m_fpUtf8_Euckr_mapper.read();
  *out2 = m_fpUtf8_Euckr_mapper.read();
}

// UTF8 문자열을 주면.. euc-kr 문자로 변환하여, 폰트에서 인덱스를 찾아서 출력
void HanDrawClass::drawString(int16_t x, int16_t y, const char* message) {
  // 굴림체 사용하며, 컬럼의 폭은 기본적으로 영숫자에 맞추어 시작점을 잡는다.
  size_t in_size = strlen(message);
  int16_t first = (int16_t)(m_dCharHalfSize / 3);
  int16_t second = (int16_t)(m_dCharHalfSize * 0.666) + 1;

  for (int i = 0; i < in_size; i++) {
    if ((message[i] & 0b10000000) > 0) { // 이건 한글이다.
      drawOneHan(x, y, message[i], message[i + 1], message[i + 2]);
      x += (int16_t)(m_dCharSize);
      i += 2;
    } else { // ascii
      // 상상도 못할 비밀.. 몇개의 특수 문자는 아스키 테이블에 매핑 해 버렸슴.
      // 게다가 폰트 데이터는 아스키 영역에 있는 것도 있고... 심지어....한글 영역에 있는 것도 있슴..
      //    (11 12 13 14       15      16     17    18    19    20          21 22 23 24 25 )
      //   0x0b c  d  e       0x0f    0x10  0x11  0x12   0x13   0x14       16 17 18  19
      //   (₁  ₂  ₃  ₄    ℉       ℃    ‰    μ    °              ¹  ²  ³  ⁴    )
      // 특별히 잘 사용할 만한 문자 모양 몇개를 아스키 테이블에 매칭 시켜 버렸슴.
      if (message[i] >= 0x0f && message[i] <= 0x14) {
        drawOneSpecial(x, y, 0xED, 0x9f , message[i]);
        x += (int16_t)(m_dCharSize);
      } else {
        switch (message[i]) {
          case 0x2E: case 0x31: case 0x3A: case 0x49: case 0x69: case 0x6c : // '.1:Iil'
            drawOneEng(x, y, first , second, message[i]);   // 상대적으로 폭이 좁은 문자는
            x += (int16_t)(second - first + 3);
            break;
          default :
            drawOneEng(x, y, 0, m_dCharHalfSize,  message[i]);
            x += (int16_t)(m_dCharHalfSize);
            break;
        }
      }
    }
    // 가로의 최대 화면 크기가 128 pixel로 가정한다.
    //if (x > 128)
    //  break;
  }
}

void HanDrawClass::drawString(int16_t x, int16_t y, char* message) {
  return drawString(x, y, (const char*)message);
}
void HanDrawClass::drawString(int16_t x, int16_t y, String message) {
  return drawString(x, y, message.c_str());
}


HanDrawClass HanDraw;
