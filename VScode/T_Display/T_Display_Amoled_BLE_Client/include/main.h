#ifndef __MAIN_H__
#define __MAIN_H__
#include <Arduino.h>
#include "lvgl.h"
#include "OneButton.h"
#include "rm67162.h"

#include <NimBLEDevice.h>
#include <MyCallbacks.h>

void click2();
void doubleclick2();
void longPressStart2();
void longPress2();
void longPressStop2();
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

void clientHandler();

#endif