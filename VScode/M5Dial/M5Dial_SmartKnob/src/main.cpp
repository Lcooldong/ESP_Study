#include <Arduino.h>
#include <WiFi.h>
#include <M5Dial.h>
#include <AS5600.h>

// PORT A
#define I2C_SDA_PIN GPIO_NUM_13
#define I2C_SCL_PIN GPIO_NUM_15

// PORT B
#define UART_RX_PIN GPIO_NUM_1
#define UART_TX_PIN GPIO_NUM_2

const unsigned long IDLE_TIMEOUT = 300; // ms

enum EncoderUIState {
  UI_IDLE,
  UI_CW,
  UI_CCW
};

int prev_x = -1;
int prev_y = -1;
int counter = 0;
int motorSpeed = 0;

AS5600 as5600;







int EncoderTask(int multiplier);
void ButtonTask();
void TouchTask();
void i2c_scan();

void setup() {

  Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin();
  // i2c_scan();
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  if(as5600.begin())
  {
    Serial.println("AS5600 not connected!");
  }
  else
  {
    Serial.println("AS5600 connected!");
  }

  auto cfg = M5.config();
  M5Dial.begin(cfg, true, true);

  if (!M5Dial.Rtc.isEnabled()) {
    Serial.println("RTC not found.");
    // M5Dial.Display.println("RTC not found.");
  }

  

  M5Dial.Display.setBrightness(100);
  M5Dial.Display.fillScreen(0x00F);
  M5Dial.Display.setTextColor(0xFFF);
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.drawString("Hello", 60, 80);
  M5Dial.Display.drawString("M5Dial!", 60, 120);

  

  Serial.print("Test");
}

void loop() {
  M5Dial.update();

  int motorDiff = EncoderTask(5);
  
  if(motorDiff != 0)
  {
    motorSpeed += motorDiff;
    motorSpeed = constrain(motorSpeed, 0, 100);
    Serial1.printf("Motor Speed: %d\r\n", motorSpeed);
  }

  
  ButtonTask();
  TouchTask();

  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 1000) 
  {
    Serial1.printf("%d\r\n", counter++);

    lastMillis = millis();
  }

}


int EncoderTask(int multiplier = 1)
{
  long newPosition = M5Dial.Encoder.read();
  unsigned long now = millis();
  static EncoderUIState uiState = UI_IDLE;
  static long oldPosition = 0;
  static unsigned long lastMoveTime = 0;

  // 1️⃣ 엔코더 움직임 감지
  if (newPosition != oldPosition) {
    int diff = newPosition - oldPosition;

    uiState = (diff > 0) ? UI_CW : UI_CCW;
    lastMoveTime = now;
    oldPosition = newPosition;

    // 화면 갱신 (움직일 때)
    M5Dial.Display.clear();
    M5Dial.Speaker.tone(8000, 20);

    M5Dial.Display.drawString(
      (uiState == UI_CW) ? "CW" : "CCW",
      M5Dial.Display.width() / 2,
      M5Dial.Display.height() / 2 - 30
    );

    M5Dial.Display.drawString(
      String(newPosition),
      M5Dial.Display.width() / 2,
      M5Dial.Display.height() / 2
    );

    Serial.println(newPosition);
    return diff * multiplier;
  }

  // 2️⃣ 일정 시간 이상 멈췄으면 IDLE
  if (uiState != UI_IDLE && (now - lastMoveTime) > IDLE_TIMEOUT) {
    uiState = UI_IDLE;

    M5Dial.Display.clear();
    M5Dial.Display.drawString(
      "IDLE",
      M5Dial.Display.width() / 2,
      M5Dial.Display.height() / 2 - 30
    );
  }
  return 0;
}

void ButtonTask()
{
  if (M5Dial.BtnA.wasPressed()) {
    M5Dial.Encoder.readAndReset();
  }
  if (M5Dial.BtnA.pressedFor(5000)) {
    M5Dial.Encoder.write(100);
  }
}

// t.state, t.x, t.y / prev_state, prev_x, prev_y
void TouchTask()
{
  static m5::touch_state_t prev_state;
  auto t = M5Dial.Touch.getDetail();
    if (prev_state != t.state) {
        prev_state = t.state;
        // static constexpr const char* state_name[16] = {
        //     "none", "touch", "touch_end", "touch_begin",
        //     "___",  "hold",  "hold_end",  "hold_begin",
        //     "___",  "flick", "flick_end", "flick_begin",
        //     "___",  "drag",  "drag_end",  "drag_begin"};
        // M5_LOGI("%s", state_name[t.state]);
        // M5Dial.Display.fillRect(0, 0, M5Dial.Display.width(),
        //                         M5Dial.Display.height() / 2, BLACK);

        // M5Dial.Display.drawString(state_name[t.state],
        //                           M5Dial.Display.width() / 2,
        //                           M5Dial.Display.height() / 2 - 30);
    }
    if (prev_x != t.x || prev_y != t.y) {
        // M5Dial.Display.fillRect(0, M5Dial.Display.height() / 2,
        //                         M5Dial.Display.width(),
        //                         M5Dial.Display.height() / 2, BLACK);
        // M5Dial.Display.drawString(
        //     "X:" + String(t.x) + " / " + "Y:" + String(t.y),
        //     M5Dial.Display.width() / 2, M5Dial.Display.height() / 2 + 30);
        prev_x = t.x;
        prev_y = t.y;
        // M5Dial.Display.drawPixel(prev_x, prev_y);
    }
}

void i2c_scan()
{
  static uint8_t i2c_counter = 0;

  for (byte i = 1; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      i2c_counter++;
      delay (10);
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (i2c_counter, DEC);
  Serial.println (" device(s).");
}

void draw_function(LovyanGFX* gfx)
{
  gfx->fillScreen(0x000);
  gfx->setTextColor(0xFFF);
  gfx->setTextSize(2);
  gfx->setTextDatum(middle_center);
  gfx->drawString("Hello", 60, 80);
  gfx->drawString("LovyanGFX!", 60, 120);
}