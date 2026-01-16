#include <Arduino.h>
// #include <WiFi.h>
#include <M5Dial.h>
#include <AS5600.h>

// #define DEBUG

// PORT A
#define I2C_SDA_PIN GPIO_NUM_13 // Yellow
#define I2C_SCL_PIN GPIO_NUM_15 // White

// PORT B
#define UART_RX_PIN GPIO_NUM_1 // White
#define UART_TX_PIN GPIO_NUM_2 // Yellow

const uint32_t IDLE_TIMEOUT = 300; // ms

enum EncoderUIState {
  UI_IDLE,
  UI_CW,
  UI_CCW
};

enum UIPage 
{
  PAGE_HOME,
  PAGE_ENCODER,
  PAGE_SETTINGS,
  PAGE_INFO
};

int numberOfMotors = 3; // 총 모터 수 3
enum MotorType { P60_KV170, TMR_57, TBM_12913 };
int motorSelection = 0;
const char *motorNames[] = { "P60_KV170", "TMR_57", "TBM_12913" };

UIPage currentPage = PAGE_HOME;
int uiSelectedItem = 0;

int prev_x = -1;
int prev_y = -1;
int counter = 0;
int motorSpeed = 0;

AS5600 as5600;
uint32_t now = 0;

int as5600_direction = 1; // 1: 시계방향, -1: 반시계방향
float angle = 0.0f;
int32_t as5600_curr_cumulative_pos = 0;
int32_t as5600_prev_cumulative_pos = 0;
const int as5600_pos_update_interval_ms = 100;
float filteredRPM = 0;
const float alpha = 0.3; // 필터 계수
const int posOffset = 2; // 위치 오차 허용 범위

static bool wasNoneZero = true;

static uint32_t lastMillis = 0;
static uint32_t lastMillis_as5600 = 0;
static uint32_t lastMillis_update = 0;
static EncoderUIState uiState = UI_IDLE;

int EncoderTask(int multiplier);
void ButtonTask();
void TouchTask();
void i2c_scan();
void updateEncoder();
void updateEncoderDisplay();
void updateSettingsDisplay();

void setup() {



  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false); // cfg, Encoder , RFID

  // // if (!M5Dial.Rtc.isEnabled()) {
  // //   Serial.println("RTC not found.");
  // //   // M5Dial.Display.println("RTC not found.");
  // // }

  M5Dial.Display.setBrightness(100);
  M5Dial.Display.fillScreen(0x002);
  M5Dial.Display.setTextColor(0xFFF);
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.drawString("Press Button", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2 - 20);
  M5Dial.Display.drawString("Below", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2 + 10);
  
  // 40 차이이로 삼각형 그리기
  M5Dial.Display.fillTriangle(
    M5Dial.Display.width() / 2 - 20, M5Dial.Display.height() / 2 + 60,
    M5Dial.Display.width() / 2 + 20, M5Dial.Display.height() / 2 + 60,
    M5Dial.Display.width() / 2, M5Dial.Display.height() / 2 + 100,
    0xF40
  );
  
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin(); // M5Dial.begin 보다 아래에 있어야함
  // i2c_scan();
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

#ifdef DEBUG
  i2c_scan();
  delay(500);
#endif
  
  if(as5600.begin())
  {
#ifdef DEBUG
    Serial1.println("AS5600 connected!");
#endif
    as5600.setDirection(AS5600_CLOCK_WISE);
  }
  else
  {
#ifdef DEBUG
    Serial1.println("AS5600 not connected!");
#endif
  }

#ifdef DEBUG
  Serial1.printf("State: %d\r\n", as5600.isConnected());
#endif

  Serial.print("Test");
}


void loop() {
  M5Dial.update();
  now = millis(); 
  int motorDiff = EncoderTask(1);



  if((now - lastMillis_as5600 >= 10) && as5600.isConnected())
  {
    counter++;
#ifdef DEBUG
    Serial1.printf("[%d]State: %d %d\r\n", counter, as5600.isConnected(), as5600.rawAngle());
#endif
    angle = as5600.rawAngle() * AS5600_RAW_TO_DEGREES;
    as5600_curr_cumulative_pos = as5600.getCumulativePosition(); // 누적 값
    // Serial1.printf("Raw Angle: %d Cumulative Pos: %d\r\n", as5600.rawAngle(), as5600_curr_cumulative_pos);

    lastMillis_as5600 = now;
  }
  else if (now - lastMillis >= 1000)
  {
    // Serial1.printf("Uptime: %lu sec\r\n", now / 1000);
    lastMillis = millis();
  }
  else if ((now - lastMillis_update >= as5600_pos_update_interval_ms))
  {
    Serial.printf("Updating UI Page: %d\r\n", currentPage);
    switch (currentPage)
    {
    case PAGE_HOME:
      
      break;
    case PAGE_ENCODER:
      updateEncoder();
      break;
    case PAGE_SETTINGS:
      if(uiState != UI_IDLE)
      {
        updateSettingsDisplay();
      }
      break;
    case PAGE_INFO:
      break;

    default:
      break;
    }
    lastMillis_update = now;
  }
  else
  {
    ButtonTask();
    TouchTask();
  }

  if(motorDiff != 0 && currentPage == PAGE_ENCODER)
  {
    motorSpeed += motorDiff;
    motorSpeed = constrain(motorSpeed, -100, 100);
    Serial1.printf("V%d\n\r", motorSpeed); // Send counter value over UART1
    wasNoneZero = true;
  }

}


int EncoderTask(int multiplier = 1)
{
  long newPosition = M5Dial.Encoder.read();

  const int detentTick = 4;

  static long oldPosition = 0;
  static int  accum = 0;        // detent 누적
  static int  value = 0;        // (선택) detent 기반 누적 값
  static int  diffValue = 0;    // 이번 detent 이벤트 변화량
  static unsigned long lastMoveTime = 0;

  int ret = 0;                  // ✅ switch에서 사용할 반환값

  // 1️⃣ 엔코더 움직임 감지
  if (newPosition != oldPosition)
  {
    int diff = (int)(newPosition - oldPosition);

    uiState = (diff > 0) ? UI_CW : UI_CCW;
    lastMoveTime = now;
    oldPosition = newPosition;

    // detent 누적
    accum += diff;

    // detent 도달 시 step 계산
    int step = accum / detentTick;
    if (step != 0)
    {
      value += step;              // 필요 없으면 제거 가능
      accum -= step * detentTick; // 잔여 보존
      diffValue = step;
    }

#ifdef DEBUG
    M5Dial.Display.clear();
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
#endif

    // 디텐트 확정 시점
    if (accum == 0 && diffValue != 0)
    {
      if (currentPage != PAGE_HOME)
        M5Dial.Speaker.tone(8000, 20);

      switch (currentPage)
      {
        case PAGE_ENCODER:
          ret = diffValue * multiplier;
          break;

        case PAGE_SETTINGS:
          motorSelection = (motorSelection + diffValue + numberOfMotors) % numberOfMotors;
          // Serial1.printf("MotorSelection: %d\r\n", motorSelection);
          break;

        default:
          break;
      }

      diffValue = 0;   // ✅ 이벤트 소비 후 리셋
    }

    // Serial1.printf("Accum: %d Value: %d diffValue: %d\r\n", accum, value, diffValue);
  }

  // 2️⃣ 일정 시간 이상 멈췄으면 IDLE
  if (uiState != UI_IDLE && (now - lastMoveTime) > IDLE_TIMEOUT)
  {
    uiState = UI_IDLE;
#ifdef DEBUG
    M5Dial.Display.clear();
    M5Dial.Display.drawString(
      "IDLE",
      M5Dial.Display.width() / 2,
      M5Dial.Display.height() / 2 - 30
    );
#endif
  }

  return ret;
}


void ButtonTask()
{
  if (M5Dial.BtnA.wasPressed()) {
    switch (currentPage)
    {
    case PAGE_HOME:
      currentPage = PAGE_ENCODER;
      break;
    case PAGE_ENCODER:
      motorSpeed = 0;
      Serial1.printf("V%d\n\r", motorSpeed);
      break;
    case PAGE_SETTINGS:
      currentPage = PAGE_ENCODER;
      motorSpeed = 0;
      Serial1.printf("V%d\n\r", motorSpeed);
      Serial1.printf("M%d\n\r", motorSelection); // Send motor selection over UART1
      updateEncoderDisplay();
#ifdef DEBUG
      Serial1.printf("Motor Selection Changed: %d\r\n", motorSelection);
#endif
      break;
    
    default:
      break;
    }
  }
  if (M5Dial.BtnA.pressedFor(1000)) {
    // ChangeMotorType Mode
    currentPage = PAGE_SETTINGS;
    motorSpeed = 0;
    Serial1.printf("V%d\n\r", motorSpeed);
    updateSettingsDisplay();
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


void updateEncoder()
{
  int32_t deltaPos = as5600_curr_cumulative_pos - as5600_prev_cumulative_pos; // 부호 중요

  if (deltaPos > 0 + posOffset) as5600_direction = 1; // 시계방향
  else if (deltaPos < 0 - posOffset) as5600_direction = -1; // 반시계방향
  else as5600_direction = 0; // 정지

  // Serial1.printf("Delta Pos: %d\r\n", deltaPos);
  float newRPM = (deltaPos / 4096.0f) * (60000.0f / as5600_pos_update_interval_ms); // 100ms 기준
  // Serial1.printf("New RPM: %.2f\r\n", newRPM);
  filteredRPM = (alpha * newRPM) + ((1 - alpha) * filteredRPM);

#ifdef DEBUG
    Serial1.printf("DIR: %2d RPM: %3.1f\r\n", as5600_direction, filteredRPM);
#endif
  bool isNoneZero = (fabs(filteredRPM) > 0.05f);
  if(isNoneZero || wasNoneZero)
  {
    updateEncoderDisplay();
  }
  wasNoneZero = isNoneZero;
  as5600_prev_cumulative_pos = as5600_curr_cumulative_pos;
}


void updateEncoderDisplay()
{
  M5Dial.Display.clear();
  M5Dial.Display.drawString(
    String(motorNames[motorSelection]),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 - 80
  );
  M5Dial.Display.drawString(
    "DIR: " + String(as5600_direction == 0 ? "STOP" : (as5600_direction > 0 ? "CW" : "CCW")),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 - 30
  );
  M5Dial.Display.drawString(
    " RPM: " + String(filteredRPM, 1),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2
  );
  M5Dial.Display.drawString(
    "Angle " + String(angle, 1),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 + 30
  );
  M5Dial.Display.drawString(
    "SPEED: " + String(motorSpeed),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 + 60
  );
}


void updateSettingsDisplay()
{
  M5Dial.Display.clear();

  // Height <- 
  int heightOffset = 0;
  switch (motorSelection)
  {
  case P60_KV170:
    heightOffset = -40;
    break;
  case TMR_57:
    heightOffset = -10;
    break;
  case TBM_12913:
    heightOffset = 20;
    break;
  default:
    break;
  }

  M5Dial.Display.fillRoundRect(
    M5Dial.Display.width() / 2 - 90,
    M5Dial.Display.height() / 2  + heightOffset,
    180,
    20,
    5,
    0x2A0
  );
  M5Dial.Display.setTextColor(0xFFF);
  M5Dial.Display.drawString(
    "P60 KV170 Motor",
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 - 30
  );
  M5Dial.Display.drawString(
    "TMR 57 Motor",
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2
  );
  M5Dial.Display.drawString(
    "TBM 12913 Motor",
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 + 30
  );
}

void i2c_scan()
{
  static uint8_t i2c_counter = 0;

  for (byte i = 1; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial1.print ("Found address: ");
      Serial1.print (i, DEC);
      Serial1.print (" (0x");
      Serial1.print (i, HEX);
      Serial1.println (")");
      i2c_counter++;
      delay (10);
      } // end of good response
  } // end of for loop
  Serial1.println ("Done.");
  Serial1.print ("Found ");
  Serial1.print (i2c_counter, DEC);
  Serial1.println (" device(s).");
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