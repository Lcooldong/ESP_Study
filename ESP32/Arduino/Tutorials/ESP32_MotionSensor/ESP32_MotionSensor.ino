#define timeSeconds 10

// Set GPIOs for LED and PIR Motion Sensor
const int led = 13;
const int motionSensor = 34;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  digitalWrite(led, HIGH);
  startTimer = true;
  lastTrigger = millis();
  Serial.print("lastTrigger : ");
  Serial.println(lastTrigger);
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  // 디지털 핀 입력 받아서 콜백함수 호출
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Set LED to LOW
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop() {
  // Current time
  now = millis();
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  // startTimer가 true 가 되었을 경우
  //timer interrupt를 발생 현재시간(now)-발생시간(last) > 정해진 시간 
  if(startTimer && (now - lastTrigger > (timeSeconds*500))) {
    Serial.println("Motion stopped...");
    digitalWrite(led, LOW);
    startTimer = false;
    Serial.print("now : ");
    Serial.println(now);
  }
  delay(1);  // core 1 충돌 방지용
}
