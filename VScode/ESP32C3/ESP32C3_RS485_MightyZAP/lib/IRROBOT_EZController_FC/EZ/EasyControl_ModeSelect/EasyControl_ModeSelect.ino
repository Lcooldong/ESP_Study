#include <IRROBOT_EZController.h>

#define ID_MAX 11

#define ModeSW_1 Easy.MODE_0.isOFF()
#define ModeSW_2 Easy.MODE_1.isOFF()
#define ModeSW_3 Easy.MODE_2.isOFF()

#define MANUAL_MODE 1
#define POS2_MODE 2
#define TOGGLE_MODE 3
#define EXT_IO_MODE 4
#define EXT_SENSING_MODE 5
#define EXT_COM_MODE 6

IRROBOT_EZController Easy(&Serial1);


void setup() {
  Easy.begin();
  Easy.MightyZap.begin(32);
  Easy.setStep(ID_MAX,0,1023);
}
void loop() 
{
  int sw_val;
  if(ModeSW_1)sw_val = 1;
  else if(ModeSW_2) sw_val = 2;
  else if(ModeSW_3) sw_val = 3;
  Easy.ModeSelect(MANUAL_MODE,POS2_MODE,TOGGLE_MODE,sw_val);
  delay(10);
}
