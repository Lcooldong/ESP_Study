#include <IRROBOT_EZController.h>

#define ID_MAX 11
#define MANUAL_POSITION_VR Easy.VR_1
#define A_POSITION_VR Easy.VR_2
#define B_POSITION_VR Easy.VR_3
#define IS_MANUAL_MODE_ON Easy.MODE_0.isOFF()
#define IS_2P_MODE_ON Easy.MODE_1.isOFF()
#define IS_TOGGLE_MODE_ON Easy.MODE_2.isOFF()
#define IS_A_POSITION_ON Easy.POS_A.isOFF()
#define IS_B_POSITION_ON Easy.POS_B.isOFF()

#define VR_MIN 0
#define VR_MAX 1023
#define VAL_MIN 0 
#define VAL_MAX 4095 
#define ID_NUM 0
#define PWM_MIN 900
#define PWM_MAX 2100
#define PWM_VAL map(position_val,VAL_MIN,VAL_MAX,PWM_MIN,PWM_MAX)

IRROBOT_EZController Easy(&Serial1);

short position_val;
bool tg_flag,Sw_status = 0;
int sw_cnt = 0, cnt = 0;

void setup() {
  Easy.begin();
  Easy.MightyZap.begin(32);
  Easy.setStep(ID_MAX,0,1023);
}
void loop() 
{
  unsigned char MightyZap_actID = ID_NUM;
  short Manual_position_val,A_stroke_val,B_stroke_val;
  int A_stroke_limit, B_stroke_limit, stroke_limit_dir;

  Manual_position_val  = map(MANUAL_POSITION_VR.read(),VR_MIN,VR_MAX,VAL_MIN,VAL_MAX);  
  A_stroke_val = map(A_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX); 
  B_stroke_val = map(B_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX);

  if(A_stroke_val>B_stroke_val)
  {
    int temp = A_stroke_limit;
    A_stroke_limit = B_stroke_val;
    B_stroke_limit = temp;
  }
  if(IS_MANUAL_MODE_ON) position_val = Manual_position_val;
  else if(IS_2P_MODE_ON)
  {
    if(IS_A_POSITION_ON) position_val = A_stroke_val;
    else if(IS_B_POSITION_ON) position_val = B_stroke_val;
  }
  else if(IS_TOGGLE_MODE_ON)
  {
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
    else {
      sw_cnt = 0;
      if(!IS_A_POSITION_ON && !IS_B_POSITION_ON){
        if(cnt++>7){
          cnt = 0;
          Sw_status = 0;
        }
      }
    }
    if(tg_flag ==1) position_val = A_stroke_val;
    else position_val = B_stroke_val;
    
  }
  Easy.MightyZap.GoalPosition(MightyZap_actID,position_val);
  Easy.servo_CH1.writeMicroseconds(PWM_VAL);
  delay(10);
}
