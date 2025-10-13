#include <Arduino.h>
#include <BleKeyboard.h>
// #include <M5Unified.h> // Memory usage is high
#include "Button2.h"

#define LED_BUILTIN  19
#define BUTTON_A_PIN 37
#define BUTTON_B_PIN 39
Button2 M5BtnA, M5BtnB;

int M5BtnAState = 0;
int M5BtnBState = 0;

BleKeyboard bleKeyboard; //("MyMacro", "M5STICK", 100); // 100% battery level

enum button_state_t : std::uint8_t
{   state_nochange, 
    state_hold, 
    state_clicked, 
    state_pressed, 
    state_released, 
    state_decide_click_count
};

void click(Button2& btn) {
    if (btn == M5BtnA) {
      Serial.println("A clicked");
      bleKeyboard.press(KEY_LEFT_CTRL);
      delay(10);
      bleKeyboard.press(KEY_TAB);
      bleKeyboard.releaseAll();
    } else if (btn == M5BtnB) {
      Serial.println("B clicked");
      bleKeyboard.press(KEY_LEFT_CTRL);
      delay(10);
      bleKeyboard.press(KEY_LEFT_SHIFT);
      delay(10);
      bleKeyboard.press(KEY_TAB);
      bleKeyboard.releaseAll();
    }
}


void setup() {
  // M5.begin();
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  pinMode(LED_BUILTIN, OUTPUT);
  
  M5BtnA.begin(BUTTON_A_PIN);
  M5BtnA.setClickHandler(click);
  M5BtnA.setDebounceTime(10);

  M5BtnB.begin(BUTTON_B_PIN);
  M5BtnB.setClickHandler(click);
  M5BtnB.setDebounceTime(10);
  bleKeyboard.begin();  
  digitalWrite(LED_BUILTIN, HIGH);

}

void loop() {

  if(bleKeyboard.isConnected()) {
    M5BtnA.loop();
    M5BtnB.loop();
  }

  // int btnAState = M5.BtnA.wasHold() ? 1
  //                 : M5.BtnA.wasClicked() ? 2 
  //                 : M5.BtnA.wasPressed() ? 3
  //                 : M5.BtnA.wasReleased() ? 4
  //                 : M5.BtnA.wasDecideClickCount() ? 5
  //                 : 0;
  // if(bleKeyboard.isConnected()) {
    
  //   switch (M5BtnAState)
  //   {
  //   case state_nochange:
      
  //     break;
  //   case state_hold:
      
  //     break;
  //   case state_clicked:
  //     Serial.printf("Button A state: %d\r\n", M5BtnAState);
  //     break;
  //   case state_pressed:
      
  //     break;
  //   case state_released:
      
  //     break;
  //   case state_decide_click_count:
      
  //     break;
  //   default:
  //     break;
  //   }
  // }
  
}

