#include "MyButton.h"
#include "MyTFT.h"

void button_init()
{
  // Button and rotary switch setup
  button.setup(BUTTON_PIN, INPUT, true); // Circuit pull-up
  button.attachClick(button_click);
  button.attachDoubleClick(doubleClick);
  button.attachLongPressStart(LongPressStart, &button);
  button.attachDuringLongPress(DuringLongPress, &button);
  button.attachLongPressStop(LongPressStop, &button);

  rotary_sw.setup(ROTARY_SW_PIN, INPUT, true); // Setup rotary switch with pull-up
  rotary_sw.attachClick(rotary_sw_click);

  
}

int click_counter = 0;
void rotary_sw_click() {
  Serial.println("Rotary switch clicked!");
  tft_printf(0, 60, "Click:%5d  ", click_counter++);
  // tft.fillScreen(TFT_BLUE); // Change screen color on rotary switch click
}

void button_tick()
{
  button.tick(); // Check button state
  rotary_sw.tick(); // Check rotary switch state
}


void button_click() {
  Serial.println("Button clicked!");
  // tft.fillScreen(TFT_RED); // Change screen color on button click
}



void doubleClick()
{
  Serial.println("Button doubleClick!");
} 

void LongPressStart(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStart()");
}

// this function will be called when the button is released.
void LongPressStop(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStop()\n");
}

// this function will be called when the button is held down.
void DuringLongPress(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - DuringLongPress()");
}


