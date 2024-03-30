#include <IRROBOT_EZController.h>

#define ID_MAX 11
#define MANUAL_POSITION_VR Easy.VR_1
#define A_POSITION_VR Easy.VR_2
#define B_POSITION_VR Easy.VR_3
#define IS_A_POSITION_ON Easyter.POS_A.isOFF()
#define IS_B_POSITION_ON Easy.POS_B.isOFF()
#define SW_A Easy.POS_A
#define SW_B Easy.POS_B

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
bool tg_flag,Sw_status = 1;
int sw_cnt = 0;

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
  int short_stroke_limit,long_stroke_limit;

  Manual_position_val  = map(MANUAL_POSITION_VR.read(),VR_MIN,VR_MAX,VAL_MIN,VAL_MAX);  
  short_stroke_limit = map(A_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX); 
  long_stroke_limit = map(B_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX);

  if(short_stroke_limit>long_stroke_limit)
  {
    int temp = short_stroke_limit;
    short_stroke_limit = long_stroke_limit;
    long_stroke_limit = temp;
  }

  if(Manual_position_val<short_stroke_limit) Manual_position_val = short_stroke_limit;
  else if(Manual_position_val>long_stroke_limit) Manual_position_val = long_stroke_limit;
  position_val = Manual_position_val;
 
  Easy.MightyZap.GoalPosition(MightyZap_actID,position_val);
  Easy.servo_CH1.writeMicroseconds(PWM_VAL);
  delay(10);
}
