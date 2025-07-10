#include <Arduino.h>
#include <BLEDevice.h>

#include "lvgl.h"
#include "OneButton.h"
#include "rm67162.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf;

uint32_t currMillis = 0;
uint32_t lastMillis = 0;

void my_disp_flush(lv_disp_drv_t *disp,
                   const lv_area_t *area,
                   lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    lcd_PushColors(area->x1, area->y1, w, h, (uint16_t *)&color_p->full);
    lv_disp_flush_ready(disp);
}

void click2();
void doubleclick2();
void longPressStart2();
void longPress2();
void longPressStop2();

OneButton button2(PIN_BUTTON_2, true, true);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // Wait for Serial to be ready
  }
  delay(1000); // Give some time for Serial to initialize
  Serial.println("T-Display Amoled Sample");
  Serial.print("ESP32 SDK: "); Serial.println(ESP.getSdkVersion());
  Serial.print("ESP32 CPU FREQ: "); Serial.print(getCpuFrequencyMhz()); Serial.println("MHz");
  Serial.print("ESP32 APB FREQ: "); Serial.print(getApbFrequency() / 1000000.0, 1); Serial.println("MHz");
  Serial.print("ESP32 FLASH SIZE: "); Serial.print(ESP.getFlashChipSize() / (1024.0 * 1024), 2); Serial.println("MB");
  Serial.print("ESP32 PSRAM SIZE: "); Serial.print(ESP.getPsramSize() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 RAM SIZE: "); Serial.print(ESP.getHeapSize() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 FREE RAM: "); Serial.print(ESP.getFreeHeap() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 MAX RAM ALLOC: "); Serial.print(ESP.getMaxAllocHeap() / 1024.0, 2); Serial.println("KB");
  Serial.print("ESP32 FREE PSRAM: "); Serial.print(ESP.getFreePsram() / 1024.0, 2); Serial.println("KB");


  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  button2.attachDuringLongPress(longPress2);

  rm67162_init(); // amoled lcd initialization

  lcd_setRotation(1);

  lv_init();

  buf = (lv_color_t *)ps_malloc(sizeof(lv_color_t) * LVGL_LCD_BUF_SIZE);
  assert(buf);


  lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_LCD_BUF_SIZE);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = EXAMPLE_LCD_H_RES;
  disp_drv.ver_res = EXAMPLE_LCD_V_RES;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  lv_obj_t *label1 = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
  lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text(label1, "#0f00ff Re-color# #00f0ff words# #f000f0 of a# label, align the lines to the center "
                    "and wrap long text automatically.");
  lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
  lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);

  lv_obj_t *label2 = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
  lv_obj_set_width(label2, 150);
  lv_label_set_text(label2, "It is a circularly scrolling text.! ");
  lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);
}

void loop()
{
    lv_timer_handler();

    currMillis = millis();
    if (currMillis - lastMillis >= 1000) {
        lastMillis = currMillis;
        Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
        // digitalWrite(PIN_LED, !digitalRead(PIN_LED)); // Toggle LED
    }
    button2.tick();
    delay(2);
}


void click2() {
  Serial.println("Button 2 click.");
}  // click2


void doubleclick2() {
  Serial.println("Button 2 doubleclick.");
}  // doubleclick2


void longPressStart2() {
  Serial.println("Button 2 longPress start");
}  // longPressStart2


void longPress2() {
  Serial.println("Button 2 longPress...");
}  // longPress2

void longPressStop2() {
  Serial.println("Button 2 longPress stop");
}  // longPressStop2