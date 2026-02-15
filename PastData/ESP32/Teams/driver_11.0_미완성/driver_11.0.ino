// not working well

// WEMOS D1 mini ESP32 

////REAR-////
//3-------4//
//---------//
//---------//
//---------//
//1-------2//
////FRONT////


#include <WiFiManager.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>


#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN   27
#define LED_COUNT 60
#define LED 2
#define MSG_BUFFER_SIZE  (50)  // null까지 포함한 길이
char msg[MSG_BUFFER_SIZE];

volatile int count0;
volatile int count1;
int totalInterrupts;
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t Task1;
TaskHandle_t Task2;

const int enable[4] = {0, 4, 12, 32};     // motor 1,2,3,4
const int driver1[4] = {5, 23, 13, 14};   // motor1 (5, 23),  motor2 (13, 14)
const int driver2[4] = {19, 18, 26, 33};  // motor3 (19, 18), motor4 (33, 26) 
 
const int cnt = sizeof(enable)/sizeof(enable[0]);
String dir;
const int echo = 16;
const int trig = 17;
float duration,  distance;
int sonarAlt = 0;
const int irPin = 21;
int ir;

const char* ssid = "IT";
const char* password = "@Polytech";
const char* mqtt_server = "broker.mqtt-dashboard.com";

boolean idle = false;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//////////////////////////////////////////////////
///////////////////////setup//////////////////////
//////////////////////////////////////////////////

void setup() {
  for(int i = 0; i < cnt; i++){
    ledcSetup(i, 5000, 8);
    ledcAttachPin(enable[i], i);
    pinMode(driver1[i], OUTPUT);
    pinMode(driver2[i], OUTPUT);
    ledcWrite(i, 0);
  }
  
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  Serial.begin(115200);
  timer_init();
  pinMode(LED, OUTPUT);
  pinMode(irPin, INPUT);
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  strip.begin(); 
  strip.show(); 
  strip.setBrightness(50);
  colorWipe(strip.Color(0,   0,   0), 0);
  colorWipe(strip.Color(  255,   0, 0), 50); // Red
  
  //wm.resetSettings(); // 와이파이 리셋
  bool res;
  res = wm.autoConnect("RemoteCar");
  
  if(!res) {
     Serial.println("Failed to connect");
     // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected");
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback); // callback 함수 연결
    colorWipe(strip.Color(  0,   0, 255), 50); // Blue
  }

  //setup_wifi();
  
  
  
  for(int i = 0; i < cnt; i++){
    ledcSetup(i, 5000, 8);
    ledcAttachPin(enable[i], i);
    pinMode(driver1[i], OUTPUT);
    pinMode(driver2[i], OUTPUT);
  }

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  xTaskCreatePinnedToCore(
    rainbowIdle,       // 태스크 함수
    "RAINBOW",           // 테스크 이름
    10000,             // 스택 크기(워드단위)
    NULL,              // 태스크 파라미터
    1,                 // 태스크 우선순위  <- 클수록 중요
    &Task1,            // 태스크 핸들
    0);                // 실행될 코어
//
  xTaskCreatePinnedToCore(
    blink1000,          // 태스크 함수
    "Task2",           // 테스크 이름
    10000,             // 스택 크기(워드단위)
    NULL,              // 태스크 파라미터
    1,                 // 태스크 우선순위
    &Task2,            // 태스크 핸들
    1);                // 실행될 코어

  idle = false;
}

//////////////////////////////////////////////////
///////////////////////loop///////////////////////
//////////////////////////////////////////////////


void loop() {

  // 연결이 끊기면 재연결
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.print("# Task loop running on core ");
  Serial.println(xPortGetCoreID());
  
  sonar();
  if (sonarAlt <= 5){
    backward();
    stop_motor();
    uturn();
    delay(1000);
  }
  if (ir == LOW){
    forward();
    stop_motor();
    delay(1000);
    
  }
  
  if (count0 > 0) {
     // Comment out enter / exit to deactivate the critical section 
     portENTER_CRITICAL(&timerMux);
     count0--;                       
     portEXIT_CRITICAL(&timerMux); 
     client.publish("MP/Polytech/position", "좌표");  // GPS 모듈 추가
  }


  // publish data
  if (count1 > 0) {
       // Comment out enter / exit to deactivate the critical section 
       portENTER_CRITICAL(&timerMux); // 카운트 시작 
       count1--;                       // 트리거 시간 감소
       portEXIT_CRITICAL(&timerMux);  // 카운트 종료
       Serial.print("# Task timer1 running on core ");
       Serial.println(xPortGetCoreID());

        
       String sonarData = "{\"distance\":\""+String(sonarAlt)+"\"}";
       sonarData.toCharArray(msg, sonarData.length()+1); //string -> char[], char* 길이는 순수한 길이, 스트링은 NULL 포함
       Serial.print("Publish message: ");
       Serial.println(msg);
       client.publish("MP/Polytech/car1", msg);
       
       totalInterrupts++; // 전체 인터럽트 횟수
//       Serial.print("totalInterrupts");
//       Serial.println(totalInterrupts);
       if ( totalInterrupts%2 == 0) {   // 짝수면 켜짐, 홀수면 꺼짐
         // Lights up the LED if the counter is even 
         digitalWrite(LED, HIGH);
       } else {
         // Then swith off
         digitalWrite(LED, LOW);
       }
  }

  

}

//////////////////////////////////////////////////
/////////////////////function/////////////////////
//////////////////////////////////////////////////

void rainbowIdle ( void *param )
{
  while (1) {
    Serial.print("# Task 1 running on core ");
    Serial.println(xPortGetCoreID());
    if(idle = true){
      Serial.println("RAINBOW");
      rainbow(2);
    }
    Serial.println(idle);
    delay(2000);
  }
}

void blink1000 ( void *param )
{
  while (1) {
    Serial.print("idle :");
    Serial.println(idle);
    delay(1000);
  }
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
  if (dir == "CCW"){
    //Serial.print(" CCW ");
    digitalWrite(tempDriver[selection], HIGH);
    digitalWrite(tempDriver[selection+1], LOW);
  }
  else if(dir == "CW"){
    //Serial.print(" CW ");
    digitalWrite(tempDriver[selection], LOW);
    digitalWrite(tempDriver[selection+1], HIGH);
  }
  ledcWrite(number-1, rate);
//  Serial.print(tempDriver[selection]);
//  Serial.print("|");
//  Serial.println(tempDriver[selection+1]);
}


void stop_motor(){
  motor(1, "CW", 0);
  motor(2, "CW", 0);
  motor(3, "CW", 0);
  motor(4, "CW", 0);
  colorWipe(strip.Color(255,   0,   0), 5); // Red
}


void forward(){
  motor(1, "CCW", 255);
  motor(2, "CCW", 255);
  motor(3, "CCW", 255);
  motor(4, "CCW", 255);
  colorWipe(strip.Color(  0, 255,   0), 5); // Green
}


void backward(){
  motor(1, "CW", 255);
  motor(2, "CW", 255);
  motor(3, "CW", 255);
  motor(4, "CW", 255);
  colorWipe(strip.Color(255,   0,   255), 3);  // purple
}


void uturn(){
  motor(1, "CCW", 255);
  motor(2, "CW", 255);
  motor(3, "CCW", 255);
  motor(4, "CW", 255);
  colorWipe(strip.Color(255,   255,   255), 7); // 약 180도
  stop_motor();
}


void right_turn(){
  motor(1, "CW", 255);
  motor(2, "CCW", 255);
  motor(3, "CW", 255);
  motor(4, "CCW", 255);
  colorWipe(strip.Color(255,   255,   0), 2);   // 약 90도
  stop_motor();
}

void left_turn(){
  motor(1, "CCW", 255);
  motor(2, "CW", 255);
  motor(3, "CCW", 255);
  motor(4, "CW", 255);
  colorWipe(strip.Color(255,   255,   0), 2);
  stop_motor();
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
  sonarAlt = constrain(sonarAlt, 0, 100);   // 2~ 500cm
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
   timerAlarmWrite(timer0, 5000000, true);
   timerAlarmWrite(timer1, 1000000, true); 
   timerAlarmEnable(timer0);
   timerAlarmEnable(timer1);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// 구독(subscribe) 받아온 payload 구현부분
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  switch((char)payload[0]){
    case '1': forward(); idle = false; break;
    case '2': left_turn(); idle = false; break;
    case '3': right_turn(); idle = false; break;
    case '4': backward(); idle = false; break;
    case '5': stop_motor(); idle = false; break;
    case '6': idle = true; break;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("MP/Polytech/car1", "reconnected");    // 재연결 브로커로 송신(publish, payload)
      // ... and resubscribe
      client.subscribe("MP/Polytech/server1");  // 재연결 구독(subscribe)->브로커에서 수신(payload 받아옴)
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
