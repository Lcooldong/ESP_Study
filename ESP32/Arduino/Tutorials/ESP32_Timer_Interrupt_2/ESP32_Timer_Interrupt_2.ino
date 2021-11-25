#include <Arduino.h>

volatile int count;    // Trigger 
int totalInterrupts;   // counts the number of triggering of the alarm

#define LED_PIN 2

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Code with critical section
void IRAM_ATTR onTime() {
   portENTER_CRITICAL_ISR(&timerMux);
   count++;
   portEXIT_CRITICAL_ISR(&timerMux);
}

// Code without critical section
/*void IRAM_ATTR onTime() {
   count++;
}*/

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

void setup() {
   Serial.begin(115200);
  
   // Configure LED output
   pinMode(LED_PIN, OUTPUT);
   digitalWrite(LED_PIN, LOW);

   timer_init();
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
