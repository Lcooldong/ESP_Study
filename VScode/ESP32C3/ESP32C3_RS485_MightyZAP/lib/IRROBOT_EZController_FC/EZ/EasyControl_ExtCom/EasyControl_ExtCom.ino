#include <IRROBOT_EZController.h>

IRROBOT_EZController Easy(&Serial1);
SoftwareSerial userSerial(8,9);

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

short position_val;
char RxChar;
short A_stroke_val,B_stroke_val;

void setup() {
  Easy.begin();
  Easy.MightyZap.begin(32);
  Easy.setStep(ID_MAX,0,1023);

  Serial.begin(9600);
  userSerial.begin(9600);
}

void loop() {
  unsigned char MightyZap_actID = ID_NUM;
  
  A_stroke_val = map(A_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX); 
  B_stroke_val = map(B_POSITION_VR.read(),  VR_MIN, VR_MAX,  VAL_MIN, VAL_MAX);
  Sw_Func();
  ExtComData_Listen();
  ExtComData_Func();
}

void Sw_Func(void){
  if(IS_A_POSITION_ON){
    userSerial.write('A');
    Serial.println("'A' Send");
    delay(500);
  }
  else if(IS_B_POSITION_ON){
    userSerial.write('B');
    Serial.println("'B' Send");
    delay(500);
  }
}

void ExtComData_Listen(void){
  if(userSerial.available()>0){
    RxChar = userSerial.read();
  }
}

bool ExtComData_Func(){
  if(RxChar == 'A'){
    position_val = A_stroke_val;
    Serial.println("'A' Recieved");
  }
  else if(RxChar == 'B'){
    position_val = B_stroke_val;
    Serial.println("'B' Recieved");
  }
  Easy.MightyZap.GoalPosition(ID_NUM, position_val);
  Easy.servo_CH1.writeMicroseconds(PWM_VAL);
  delay(10);
}
