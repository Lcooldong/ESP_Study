#ifndef __MY_BUTTON_H__
#define __MY_BUTTON_H__

#include <Arduino.h>
#include <OneButton.h>
#include "main.h"


static OneButton button; // Button on GPIO 0, pull-up enabled
static OneButton rotary_sw;

void button_init();
void button_click();
void rotary_sw_click();
void button_tick();

void doubleClick();
void LongPressStart(void *oneButton);
void LongPressStop(void *oneButton);
void DuringLongPress(void *oneButton);

#endif

