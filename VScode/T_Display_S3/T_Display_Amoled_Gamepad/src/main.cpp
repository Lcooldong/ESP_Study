#include <Arduino.h>
#include <BleGamepad.h>
#include <OneButton.h>
#include "pins_config.h"


#define NUM_BUTTONS 13

//ABXY BUTTONS
#define X_BUTTON 15         // A
#define CIRCLE_BUTTON 17    // B
#define TRIANGLE_BUTTON 4  // Y
#define SQUARE_BUTTON 5    // X

//TRIGGERS
#define R1_BUTTON 18
#define R2_BUTTON 19
#define L1_BUTTON 21
#define L2_BUTTON 22


//MENU BUTTONS
// #define START_BUTTON 23 // Redefine
// #define SELECT_BUTTON 26
// #define PS_BUTTON 25

//JOYSTICKS BUTTONS
#define R3_BUTTON 33
#define L3_BUTTON 32

typedef enum{ANDROID, PS1, PC} GamepadModes;
GamepadModes gamepadMode = PC;


BleGamepad bleGamepad("BLE Gamepad", "ESP32", 100);
BleGamepadConfiguration bleGamepadConfig;
// OneButton button2(PIN_BUTTON_2, true, true);

int button2State = 0;
uint32_t currMillis = 0;
uint32_t lastMillis[4] = {0,};
int counter = 0;

// int buttonsPins[NUM_BUTTONS] = {X_BUTTON, CIRCLE_BUTTON, TRIANGLE_BUTTON, SQUARE_BUTTON,
//                           R1_BUTTON, R2_BUTTON, L1_BUTTON, L2_BUTTON,
//                           START_BUTTON, SELECT_BUTTON, PS_BUTTON,
//                           R3_BUTTON, L3_BUTTON};

int androidGamepadButtons[NUM_BUTTONS] = {1, 2, 3, 4, 8, 10, 7, 9, 12, 11, 13, 15, 14};
int PS1GamepadButtons[NUM_BUTTONS] = {2, 3, 4, 1, 6, 8, 5, 7, 10, 9, 13, 12, 11};
int PCGamepadButtons[NUM_BUTTONS] = {1, 2, 4, 3, 6, 8, 5, 7, 10, 9, 0, 12, 11};

void click2();

void setup() {
  Serial.begin(115200);
  // button2.attachClick(click2);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  // while (!Serial) {
  //   // Wait for Serial to be ready
  // }

  delay(1000); // Give some time for Serial to initialize
  Serial.println("BLE Gamepad Example");
  
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepadConfig.setAutoReport(false);
  // bleGamepadConfig.setVid(0xe502);
  // bleGamepadConfig.setPid(0xabcd);
  // bleGamepadConfig.setHatSwitchCount(4);
  bleGamepadConfig.setTXPowerLevel(-3); // -12, -9, -6, -3, 0, 3, 6 and 9 ? Not working 
  bleGamepad.begin(&bleGamepadConfig);
  Serial.println("BLE Gamepad Initialized");
  Serial.printf("Power Level: %d\n", bleGamepad.getTXPowerLevel());

}

void loop() {
  if(bleGamepad.isConnected()) {
    switch (gamepadMode)
    {
    case ANDROID:
      break;
    case PS1:
      break;
    case PC:
      if(!digitalRead(PIN_BUTTON_2)) {
        bleGamepad.press(PCGamepadButtons[0]); 
        digitalWrite(PIN_LED, HIGH);
        delay(5);
      } else{
        bleGamepad.release(PCGamepadButtons[0]);
        digitalWrite(PIN_LED, LOW);
      }
      break;
    default:
      break;
    }

    bleGamepad.sendReport();
  }
 
  currMillis = millis();
  if (currMillis - lastMillis[0] >= 1000) {
    lastMillis[0] = currMillis;
    counter++;
    Serial.printf("[%d] Free heap: %d\n", counter, ESP.getFreeHeap());
  }
  else if(currMillis - lastMillis[1] >= 2) {
    lastMillis[1] = currMillis;
    // button2.tick(); // Update the button state
  }
}

void click2() {
  Serial.println("Button 2 click.");
  button2State = true;
}  // click1