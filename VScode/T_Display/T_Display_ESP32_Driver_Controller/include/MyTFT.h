#ifndef __MY_TFT_H__
#define __MY_TFT_H__

#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>


#define MAX_IMAGE_WIDTH 240 // Adjust for your images
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))
#define TJPGD_WORKSPACE_SIZE (3500 + 6144)

const int NUM_ITEM = 4; // Number of menu items
const int MAX_ITEM_LENGTH = 20; // Maximum length of each menu item

enum {
  First_MENU = 0,
  Second_MENU = 1,
  Third_MENU = 2,
  Fourth_MENU = 3,
};


static TFT_eSPI tft = TFT_eSPI(); // Invoke library
static TJpg_Decoder tjpg = TJpg_Decoder();

static char menu_items[NUM_ITEM][MAX_ITEM_LENGTH] = {{"FIRST MENU"}, {"SECOND MENU"}, {"THIRD MENU"}, {"FOURTH MENU"}};
static int item_selected = 0; // Current menu index
static int current_screen = 0;
static int item_sel_previous;
static int item_sel_next;



void tft_init();
void drawJpg(int16_t x, int16_t y, const char *pFilename);
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
void tft_printf(int16_t x, int16_t y, const char *format, ...);
void tft_clear(int16_t TFT_COLOR);

#endif