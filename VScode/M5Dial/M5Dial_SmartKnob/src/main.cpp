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


int prev_x = -1;
int prev_y = -1;
long oldPosition = -999;

int counter = 0;

static m5::touch_state_t prev_state;
AS5600 as5600;

void EncoderTask();
void ButtonTask();
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


  EncoderTask();
  ButtonTask();

  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 1000) 
  {
    Serial1.printf("%d\r\n", counter++);

    lastMillis = millis();
  }

}


void EncoderTask()
{
  long newPosition = M5Dial.Encoder.read();
  if (newPosition != oldPosition) {
      M5Dial.Speaker.tone(8000, 20);
      M5Dial.Display.clear();
      oldPosition = newPosition;
      Serial.println(newPosition);
      M5Dial.Display.drawString(String(newPosition),
                                M5Dial.Display.width() / 2,
                                M5Dial.Display.height() / 2);
  }
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