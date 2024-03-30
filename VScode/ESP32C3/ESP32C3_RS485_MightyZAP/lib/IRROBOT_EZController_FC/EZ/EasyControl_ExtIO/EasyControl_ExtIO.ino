#include <IRROBOT_EZController.h>

#define ID_MAX 11
#define A_POSITION_VR Easy.VR_2
#define B_POSITION_VR Easy.VR_3
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

void setup() {
  pinMode(7,INPUT);
  pinMode(11,INPUT);
  Easy.begin();
  Easy.MightyZap.begin(32);
  Easy.setStep(ID_MAX,0,1023);
}
void loop() {
  unsigned char MightyZap_actID = ID_NUM;
  short A_stroke_val,B_stroke_val;
 
  A_stroke_val = map(A_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX); 
  B_stroke_val = map(B_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX);

  if(digitalRead(7) == HIGH) position_val = A_stroke_val;
  else if(digitalRead(11) == HIGH) position_val = B_stroke_val;
  else position_val = position_val;
  Easy.MightyZap.GoalPosition(MightyZap_actID,position_val);
  Easy.servo_CH1.writeMicroseconds(PWM_VAL);
  delay(10);
}
