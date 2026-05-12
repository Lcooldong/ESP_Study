#include <Arduino.h>
#include "Button.h"

const int MYBUTTON_PIN = D8;
const int HOLDONG_BUTTON_PIN = D9;
const int BRAKE_BUTTON_PIN = D10;
const int LED_PIN = D2;

// UART Pins + USB-C(upload + boot) include
const int UART_RX_PIN = D7;
const int UART_TX_PIN = D6; 

uint32_t counter = 0;
uint32_t lastIndicatorTime = 0;


Button myBtn(MYBUTTON_PIN, LOW, 10);  
Button holdBtn(HOLDONG_BUTTON_PIN, HIGH, 10); // High -> Holding
Button brakeBtn(BRAKE_BUTTON_PIN, LOW, 50);


bool ledState = LOW;
uint8_t ledValue = 0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);  // Turn on

  Serial.println("Button Test Started");
}

void loop() {
  myBtn.read();
  holdBtn.read();
  brakeBtn.read();

  uint32_t currentTime = millis();
  if (currentTime - lastIndicatorTime >= 1000) {
    counter++;
    Serial.printf("Indicator Serial: Loop is running... (Count: %lu)\n\r", counter);
    Serial1.printf("Indicator UART: Loop is running... (Count: %lu)\n\r", counter);
    lastIndicatorTime = currentTime;
  }

  if(myBtn.wasPressed())
  {
    Serial.println("MyButton Pressed");
    ledState = !ledState;
    ledValue = ledState ? 255 : 0;
    Serial.printf("LED Value: %d\n", ledValue);
    analogWrite(LED_PIN, ledValue);
  }

  if(holdBtn.pressedFor(1000))
  {
    Serial.println("Hold Button Pressed for 1 second");
  }
  else if(holdBtn.wasReleased())
  {
    Serial.println("Hold Button Released");
  }
  
  if(brakeBtn.wasReleased())
  {
    Serial.println("Brake Button Released");
  }
  

}


