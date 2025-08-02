#include <Arduino.h>
#include <M5Unified.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>

#include "wavefile.h"

uint32_t currentMillis = 0;
uint32_t previousMillis[4] = {0,};

#define IR_LED         9
#define RED_LED        10
#define LED_CHANNEL    0
#define LED_FREQUENCY  5000
#define LED_RESOLUTION 8

enum ir_LG2_name
{
  TURN_ON,
  TURN_OFF,
  
};

const uint64_t ir_LG2_command[4][8] = {
  {0x8801C0D},
  {0x88C0051},
  {},
  {}
};





// 바람 1칸  8808A13
// 바람 2칸  8808A02
// 바람 3칸  8808A24
// 바람 4칸  8808A46
// 바람 ~칸  8808A57


// AI  모드  880B252
// 제습 모드 8809913
// 송풍 모드 880A341

// 18'c 880831C
// 19'c 880841D
// 20'c 880851E
// 21'c 880861F
// 22'c 8808710
// 23'c 8808811
// 24'c 8808912
// 25'c 8808A13
// 26'c 8808B14
// 27'c 8808C15
// 28'c 8808D16
// 29'c 8808E17
// 30'c 8808F18
// +0x0000 0101

#define BASE_TEMP 880831C

const uint64_t ir_LG2_temperature[10][8] = {
  0,
};




IRsend irsend(IR_LED);


void setup(void)
{
  // auto cfg = M5.config();
  // cfg.serial_baudrate = 115200;
  Serial.begin(115200);
  irsend.begin();
  M5.begin();
  ledcAttachPin(RED_LED, LED_CHANNEL);
  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
  ledcWrite(LED_CHANNEL, 50);
  // M5.begin(cfg);

  while(!Serial){};
  M5.Lcd.setTextColor(3);
  /// For models with EPD : refresh control
  M5.Display.setEpdMode(epd_mode_t::epd_fastest); // fastest but very-low quality.

  // if (M5.Display.width() < M5.Display.height())
  // { /// Landscape mode.
  //   M5.Display.setRotation(M5.Display.getRotation() ^ 1);
  // }

  if (M5.Speaker.isEnabled())
  {
    /// set master volume (0~255)
    M5.Speaker.setVolume(100);
    /// play beep sound 2000Hz 100msec (background task)
    M5.Speaker.tone(2000, 100);
    /// wait done
    while (M5.Speaker.isPlaying()) { M5.delay(1); }
    /// play beep sound 1000Hz 100msec (background task)
    M5.Speaker.tone(1000, 100);
    /// wait play beep sound 2000Hz 100msec (background task)
    while (M5.Speaker.isPlaying()) { M5.delay(1); }
    // M5.Speaker.playRaw(wav_8bit_44100, sizeof(wav_8bit_44100), 44100, false);
  }

  
}

void loop(void)
{
  currentMillis = millis();
  M5.delay(1);
  M5.update();

  static constexpr const int colors[] = { TFT_WHITE, TFT_CYAN, TFT_RED, TFT_YELLOW, TFT_BLUE, TFT_GREEN };
  static constexpr const char* const names[] = { "none", "wasHold", "wasClicked", "wasPressed", "wasReleased", "wasDecideCount" };

  int w = M5.Display.width() / 5;
  int h = M5.Display.height();
  M5.Display.startWrite();

  /// BtnPWR: "wasClicked"/"wasHold"  can be use.
  /// BtnPWR of CoreInk: "isPressed"/"wasPressed"/"isReleased"/"wasReleased"/"wasClicked"/"wasHold"/"isHolding"  can be use.
  int state = M5.BtnPWR.wasHold() ? 1
            : M5.BtnPWR.wasClicked() ? 2
            : M5.BtnPWR.wasPressed() ? 3
            : M5.BtnPWR.wasReleased() ? 4
            : M5.BtnPWR.wasDecideClickCount() ? 5
            : 0;

  if (state)
  {
    M5_LOGI("BtnPWR:%s  count:%d", names[state], M5.BtnPWR.getClickCount());
    Serial.printf("BtnPWR:%s  count:%d\r\n", names[state], M5.BtnPWR.getClickCount());
    // M5.Display.fillRect(w*0, 0, w-1, h, colors[state]);

    switch (state)
    {
    case 1: 
      irsend.sendLG2(0x88C0051, 28, 2); // Hold -> Turn off
      break;
    case 2:
      
    default:
      break;
    }
  }

  /// BtnA,BtnB,BtnC,BtnEXT: "isPressed"/"wasPressed"/"isReleased"/"wasReleased"/"wasClicked"/"wasHold"/"isHolding"  can be use.
  state = M5.BtnA.wasHold() ? 1
        : M5.BtnA.wasClicked() ? 2
        : M5.BtnA.wasPressed() ? 3
        : M5.BtnA.wasReleased() ? 4
        : M5.BtnA.wasDecideClickCount() ? 5
        : 0;
  if (state)
  {
    M5_LOGI("BtnA:%s  count:%d", names[state], M5.BtnA.getClickCount());
    Serial.printf("Button A\r\n");
    // M5.Display.fillRect(w*1, 0, w-1, h, colors[state]);

    switch (state)
    {
    case 1: // clicked
      // irsend.sendLG2(0x8801C0D, 28, 2);
      break;
    case 2:
      irsend.sendLG2(0x880190A, 28, 2); // 0x8801C0D 동일
    default:
      break;
    }
  }



  state = M5.BtnB.wasHold() ? 1
        : M5.BtnB.wasClicked() ? 2
        : M5.BtnB.wasPressed() ? 3
        : M5.BtnB.wasReleased() ? 4
        : M5.BtnB.wasDecideClickCount() ? 5
        : 0;
  if (state)
  {
    M5_LOGI("BtnB:%s  count:%d", names[state], M5.BtnB.getClickCount());
    // M5.Display.fillRect(w*2, 0, w-1, h, colors[state]);
    Serial.printf("Button B\r\n");

    switch (state)
    {
    case 2: // clicked
      irsend.sendLG2(0x8801C0D, 28, 2);

      break;
    
    default:
      break;
    }

  }
  M5.Display.endWrite();

  if(!M5.Display.displayBusy())
  {
    static uint32_t prev_sec;
    uint32_t sec = m5gfx::millis()/1000;
    if(prev_sec != sec)
    {
      prev_sec = sec;

      static int prev_battery = INT_MAX;
      int battery = M5.Power.getBatteryLevel();
      if (prev_battery != battery)
      {
        prev_battery = battery;
        M5.Display.startWrite();
        
        M5.Display.setTextColor(M5.Display.color24to16(0xFFFFFF), M5.Display.color24to16(0x000000));
        M5.Display.setTextSize(2);
        // M5.Display.setCursor(0, M5.Display.fontHeight() * 3);
        M5.Display.setCursor(M5.Display.width()-3, 0); // Bat:%03d


        M5.Display.print("Bat:");
        // M5.Display.drawString("Bat", 0, M5.Display.fontHeight() * 3);
        if (battery >= 0)
        {
          M5.Display.printf("%03d", battery);
        }
        else
        {
          M5.Display.print("none");
        }
        M5.Display.endWrite();
      }


    }
  }

}
