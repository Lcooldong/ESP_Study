#include <Arduino.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <OneButton.h>

#include <HardwareSerial.h>
#include <ACAN_ESP32.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <core_version.h> // For ARDUINO_ESP32_RELEASE


#define BUTTON_PIN GPIO_NUM_35 // GPIO pin for the button

#define ROTARY_CLK_PIN GPIO_NUM_32
#define ROTARY_DT_PIN  GPIO_NUM_33
#define ROTARY_SW_PIN  GPIO_NUM_12

#define CAN_RX_PIN GPIO_NUM_37 // CAN RX pin
#define CAN_TX_PIN GPIO_NUM_38 // CAN TX pin

#define LED_CHANNEL 0 // LED channel for PWM (not used in this example)
#define LED_FREQUENCY 5000 // Frequency for PWM (not used in this example)
#define LED_RESOLUTION 8 // Resolution for PWM (not used in this example)
#define LED_PIN GPIO_NUM_17 // GPIO pin for the LED (not used in this example)

static const uint32_t DESIRED_BIT_RATE = 500UL * 1000UL ; // 500 kb/s

uint32_t currentMillis = 0;
uint32_t previousMillis[4] = {0,};
int counter = 0;
uint8_t ledValue = 0;

TFT_eSPI tft = TFT_eSPI(); // Invoke library
OneButton button; // Button on GPIO 0, pull-up enabled
OneButton rotarySW;

HardwareSerial Serail1(1); // Use UART1 for serial communication

void buttonClick() {
  Serial.println("Button clicked!");
  // tft.fillScreen(TFT_RED); // Change screen color on button click
}
void rotarySWClick() {
  Serial.println("Rotary switch clicked!");
  // tft.fillScreen(TFT_BLUE); // Change screen color on rotary switch click
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, GPIO_NUM_13, GPIO_NUM_15); // UART1 with RX on GPIO 16 and TX on GPIO 17
  ledcAttachPin(LED_PIN, LED_CHANNEL); // Attach LED pin to channel (not used in this example)
  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION); // Setup LED channel (not used in this example)
  ledcWrite(LED_CHANNEL, 0); // Set LED brightness to 50% (not used in this example)

  // Button and rotary switch setup
  button.setup(BUTTON_PIN, INPUT, true); // Circuit pull-up
  button.attachClick(buttonClick);
  rotarySW.setup(ROTARY_SW_PIN, INPUT_PULLUP, true); // Setup rotary switch with pull-up
  rotarySW.attachClick(rotarySWClick);

  pinMode(ROTARY_CLK_PIN, INPUT_PULLUP); // Set rotary encoder CLK pin as input with pull-up
  pinMode(ROTARY_DT_PIN, INPUT_PULLUP); // Set rotary encoder DT pin

  ACAN_ESP32_Settings settings (DESIRED_BIT_RATE) ;
  settings.mRxPin = CAN_RX_PIN ; 
  settings.mTxPin = CAN_TX_PIN ;
  settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode ;
  const uint32_t errorCode = ACAN_ESP32::can.begin (settings) ;

  if (errorCode == 0) {
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
  }else {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

  delay(3000); 

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  } else {
    Serial.println("LittleFS mounted!");
  }

  Serial.printf("FLASH: %d MB\r\n", ESP.getFlashChipSize()/(1024*1024)); // Should print 16777216 for 16MB

  tft.begin(); // Initialize TFT
  tft.setRotation(1); // Set rotation


}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis[0] >= 1000) { // Update every second
    previousMillis[0] = currentMillis;
    counter++;
    Serial.printf("Counter: %d %d\r\n", counter, digitalRead(ROTARY_SW_PIN)); // Print counter and button state to serial
    Serial1.printf("Serial1: %d\r\n", counter); // Send counter value over UART1

    tft.fillScreen(TFT_BLACK); // Clear screen
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color and background
    tft.setCursor(10, 10); // Set cursor position
    tft.setTextSize(1); // Set text size
    tft.printf("[%d]", counter); // Print message
  }
  else if (currentMillis - previousMillis[1] >= 10) { // Update every 500ms
    previousMillis[1] = currentMillis;
    button.tick(); // Check button state
    rotarySW.tick(); // Check rotary switch state
  }
  if(Serial.available()) {
    char c = Serial.read();
    switch (c)
    {
    case '1': // Example command to change text color
      ledValue++;
      break;
    case '2': // Example command to change text color
      ledValue--;
      break;
    default:
      break;
    }
    ledcWrite(LED_CHANNEL, ledValue); // Set LED brightness to 50% (not used in this example)
    Serial.printf("LED Value: %d\r\n", ledValue); // Print LED value to serial
  }

}

