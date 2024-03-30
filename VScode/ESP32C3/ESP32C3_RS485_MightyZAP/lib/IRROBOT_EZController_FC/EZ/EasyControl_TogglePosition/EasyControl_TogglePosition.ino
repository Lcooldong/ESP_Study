#include <IRROBOT_EZController.h>

#define ID_MAX 11
#define A_POSITION_VR Easy.VR_2
#define B_POSITION_VR Easy.VR_3
#define VR_MIN 0
#define VR_MAX 1023
#define VAL_MIN 0 
#define VAL_MAX 4095
#define IS_A_POSITION_ON Easy.POS_A.isOFF()
#define IS_B_POSITION_ON Easy.POS_B.isOFF()
#define ID_NUM 0
#define PWM_MIN 900
#define PWM_MAX 2100
#define PWM_VAL map(position_val,VAL_MIN,VAL_MAX,PWM_MIN,PWM_MAX)

IRROBOT_EZController Easy(&Serial1);

short position_val;
bool tg_flag,Sw_status = 0;
int sw_cnt = 0,cnt =0 ;

void setup() {
  Easy.begin();
  Easy.MightyZap.begin(32);
  Easy.setStep(ID_MAX,0,1023);
}
void loop() {
  unsigned char MightyZap_actID = ID_NUM;
  short A_stroke_val;
  short B_stroke_val;
 
  A_stroke_val = map(A_POSITION_VR.read(),VR_MIN,VR_MAX,VAL_MIN,VAL_MAX); 
  B_stroke_val = map(B_POSITION_VR.read(),VR_MIN,VR_MAX,VAL_MIN,VAL_MAX);

  if(IS_A_POSITION_ON || IS_B_POSITION_ON){
    if(!Sw_status){
      if(sw_cnt++>7){
        tg_flag ^= 1;
        Sw_status = 1;
        sw_cnt = 0;
      }
    }
    else sw_cnt = 0;
  }
  else{
    sw_cnt = 0;
    if(!IS_A_POSITION_ON && !IS_B_POSITION_ON){
      if(cnt++>7){
       cnt = 0;
       Sw_status = 0;
      }
    }
  }
  if(tg_flag) position_val = A_stroke_val;
  else position_val = B_stroke_val;

  Easy.MightyZap.GoalPosition(MightyZap_actID,position_val);
  Easy.servo_CH1.writeMicroseconds(PWM_VAL);
  delay(10);
}
