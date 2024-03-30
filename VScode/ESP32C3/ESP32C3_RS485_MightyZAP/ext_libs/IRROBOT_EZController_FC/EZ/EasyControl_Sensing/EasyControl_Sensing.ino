#include <IRROBOT_EZController.h>

IRROBOT_EZController Easy(&Serial1);

#define ID_MAX 11
#define MANUAL_POSITION_VR Easy.VR_1
#define A_POSITION_VR Easy.VR_2
#define B_POSITION_VR Easy.VR_3
#define EXT_ANALOG_VR Easy.VR_4   //VR4 : A0  //VR5 : A2  //VR6 : A3
#define VR_MIN 0
#define VR_MAX 1023
#define VAL_MIN 0
#define VAL_MAX 4095
#define ID_NUM 0
#define PWM_MIN 900
#define PWM_MAX 2100
#define PWM_VAL map(position_val,VAL_MIN,VAL_MAX,PWM_MIN,PWM_MAX)

short position_val;

void setup(){
  Easy.begin();
  Easy.MightyZap.begin(32);
  Easy.setStep(ID_MAX, 0, 1023);
}

void loop() {
  unsigned char MightyZap_actID = ID_NUM;
  short Ext_analog_val;

  Ext_analog_val = map(EXT_ANALOG_VR.read(), VR_MIN, VR_MAX, VAL_MIN, VAL_MAX);
  position_val = Ext_analog_val;
  Easy.MightyZap.GoalPosition(ID_NUM, position_val);
  Easy.servo_CH1.writeMicroseconds(PWM_VAL);
  delay(10);
}
