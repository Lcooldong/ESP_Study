// WEMOS D1 mini ESP32 

////REAR-////
//4-------3//
//---------//
//---------//
//---------//
//1-------2//
////FRONT////

volatile int count;
int totalInterrupts;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#define LED_PIN 2

const int enable[4] = {0, 4, 12, 32};     // motor 1,2,3,4
const int driver1[4] = {5, 23, 19, 18};   // motor1 (5, 23),  motor2 (19, 18)
const int driver2[4] = {13, 14, 33, 26};  // motor3 (13, 14), motor4 (33, 26)  
const int cnt = sizeof(enable)/sizeof(enable[0]);
String dir;
const int echo = 16;
const int trig = 17;

void IRAM_ATTR onTime() {
   portENTER_CRITICAL_ISR(&timerMux);
   count++;
   portEXIT_CRITICAL_ISR(&timerMux);
}
void timer_init(){
   // Configure the Prescaler at 80 the quarter of the ESP32 is cadence at 80Mhz
   // 80000000 / 80 = 1000000 tics / seconds  T: 0.000001s, 1초에 1,000,000 찍음
   timer = timerBegin(0, 80, true);                
   timerAttachInterrupt(timer, &onTime, true);    
    
   // Sets an alarm to sound every second
   //  number/1,000,000 -> 시간(s)   또는  원하는 시간/0.000001 -> 타이머 인터럽트가 발생되는 Count횟수
   timerAlarmWrite(timer, 2000000, true); 
   timerAlarmEnable(timer);
}


void motor(int number, char* dir, int rate){
  
  int selection;
  const int* tempDriver;
  
  if(number >=3){
   tempDriver = driver2;
  }
  else{
    tempDriver = driver1;
  }
  
  if (number % 2 == 0)selection = 2;
  else selection = 0;
  
  digitalWrite(enable[number-1], LOW);
  if (dir == "CW"){
    Serial.print(" CW ");
    digitalWrite(tempDriver[selection], HIGH);
    digitalWrite(tempDriver[selection+1], LOW);
  }
  else if(dir == "CCW"){
    Serial.print(" CCW ");
    digitalWrite(tempDriver[selection], LOW);
    digitalWrite(tempDriver[selection+1], HIGH);
  }
  ledcWrite(number, rate);
  Serial.print(tempDriver[selection]);
  Serial.print("|");
  Serial.println(tempDriver[selection+1]);
}




void setup() {
  Serial.begin(115200);
  timer_init();
  for(int i = 0; i < cnt; i++){
    ledcSetup(i, 5000, 8);
    ledcAttachPin(enable[i], i);
    pinMode(driver1[i], OUTPUT);
    pinMode(driver2[i], OUTPUT);
  }

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  
  motor(3, "CCW", 255);

}

void loop() {
  if (count > 0) {
       // Comment out enter / exit to deactivate the critical section 
       portENTER_CRITICAL(&timerMux); // 카운트 시작 
       count--;                       // 트리거 시간 감소
       portEXIT_CRITICAL(&timerMux);  // 카운트 종료

       totalInterrupts++; // 전체 인터럽트 횟수
       Serial.print("totalInterrupts");
       Serial.println(totalInterrupts);
       if ( totalInterrupts%2 == 0) {   // 짝수면 켜짐, 홀수면 꺼짐
         // Lights up the LED if the counter is even 
         digitalWrite(LED_PIN, HIGH);
       } else {
         // Then swith off
         digitalWrite(LED_PIN, LOW);
       }
    }
}

void stop_motor(){
  
}

void forward(){
  
}

void turn(){
  
}
