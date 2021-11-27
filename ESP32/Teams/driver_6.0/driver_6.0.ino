// WEMOS D1 mini ESP32 

////REAR-////
//3-------4//
//---------//
//---------//
//---------//
//1-------2//
////FRONT////

volatile int count0;
volatile int count1;
int totalInterrupts;
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#define LED 2

const int enable[4] = {0, 4, 12, 32};     // motor 1,2,3,4
const int driver1[4] = {5, 23, 13, 14};   // motor1 (5, 23),  motor2 (13, 14)
const int driver2[4] = {19, 18, 26, 33};  // motor3 (19, 18), motor4 (33, 26) 
 
const int cnt = sizeof(enable)/sizeof(enable[0]);
String dir;
const int echo = 16;
const int trig = 17;
float duration,  distance;
int sonarAlt = 0;


void setup() {
  Serial.begin(115200);
  timer_init();
  pinMode(LED, OUTPUT);
  
  for(int i = 0; i < cnt; i++){
    ledcSetup(i, 5000, 8);
    ledcAttachPin(enable[i], i);
    pinMode(driver1[i], OUTPUT);
    pinMode(driver2[i], OUTPUT);
  }

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  stop_motor();
  
}

void loop() {
  
  //Serial.println(sonarAlt);
  
  if (count0 > 0) {
       // Comment out enter / exit to deactivate the critical section 
       portENTER_CRITICAL(&timerMux); // 카운트 시작 
       count0--;                       // 트리거 시간 감소
       portEXIT_CRITICAL(&timerMux);  // 카운트 종료

       totalInterrupts++; // 전체 인터럽트 횟수
       Serial.print("totalInterrupts");
       Serial.println(totalInterrupts);
       Serial.print("distance : ");
       Serial.println(sonarAlt);
       if ( totalInterrupts%2 == 0) {   // 짝수면 켜짐, 홀수면 꺼짐
         // Lights up the LED if the counter is even 
         digitalWrite(LED, HIGH);
       } else {
         // Then swith off
         digitalWrite(LED, LOW);
       }
  }

  sonar();

}

//////////////////////////////////////////////////
/////////////////////function/////////////////////
//////////////////////////////////////////////////

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
  if (dir == "CCW"){
    Serial.print(" CCW ");
    digitalWrite(tempDriver[selection], HIGH);
    digitalWrite(tempDriver[selection+1], LOW);
  }
  else if(dir == "CW"){
    Serial.print(" CW ");
    digitalWrite(tempDriver[selection], LOW);
    digitalWrite(tempDriver[selection+1], HIGH);
  }
  ledcWrite(number-1, rate);
  Serial.print(tempDriver[selection]);
  Serial.print("|");
  Serial.println(tempDriver[selection+1]);
}


void stop_motor(){
  motor(1, "CW", 0);
  motor(2, "CW", 0);
  motor(3, "CW", 0);
  motor(4, "CW", 0);
}


void forward(){
  motor(1, "CCW", 255);
  motor(2, "CCW", 255);
  motor(3, "CCW", 255);
  motor(4, "CCW", 255);
}


void backward(){
  motor(1, "CW", 255);
  motor(2, "CW", 255);
  motor(3, "CW", 255);
  motor(4, "CW", 255);
}


void uturn(){
  motor(1, "CCW", 255);
  motor(2, "CW", 255);
  motor(3, "CCW", 255);
  motor(4, "CW", 255);
  delay(510);
}


void turn(){
  
}

void sonar(){
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);  
  duration = pulseIn (echo, HIGH);
  distance = ((float)(340.0 *  duration) / 10000.0) / 2.0;
  sonarAlt = (int)distance;//  + offset;
  sonarAlt = constrain(sonarAlt, 0, 500);
}


void IRAM_ATTR onTimer0() { // 타이머 인터럽트 결과
   portENTER_CRITICAL_ISR(&timerMux); 
   count0++;
   portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timerMux);
   count1++;
   portEXIT_CRITICAL_ISR(&timerMux);
}


void timer_init(){
   // Configure the Prescaler at 80 the quarter of the ESP32 is cadence at 80Mhz
   // 80000000 / 80 = 1000000 tics / seconds  T: 0.000001s, 1초에 1,000,000 찍음
   timer0 = timerBegin(0, 80, true); 
   timer1 = timerBegin(1, 80, true);                
   timerAttachInterrupt(timer0, &onTimer0, true);
   timerAttachInterrupt(timer1, &onTimer1, true);

       
    
   // Sets an alarm to sound every second
   //  number/1,000,000 -> 시간(s)   또는  원하는 시간/0.000001 -> 타이머 인터럽트가 발생되는 Count횟수
   timerAlarmWrite(timer0, 500000, true);
   timerAlarmWrite(timer1, 2000000, true); 
   timerAlarmEnable(timer0);
   timerAlarmEnable(timer1);
}
