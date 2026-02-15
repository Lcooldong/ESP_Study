// WEMOS D1 mini ESP32 

////REAR-////
//3-------4//
//---------//
//---------//
//---------//
//1-------2//
////FRONT////
#define LED 2
#include <WiFiManager.h>
#include <WiFi.h>
#include <PubSubClient.h>

volatile int count0;
volatile int count1;
int totalInterrupts;
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

const int enable[4] = {0, 4, 12, 32};     // motor 1,2,3,4
const int driver1[4] = {5, 23, 13, 14};   // motor1 (5, 23),  motor2 (13, 14)
const int driver2[4] = {19, 18, 26, 33};  // motor3 (19, 18), motor4 (33, 26)
#define SONAR_BUFFER_SIZE  (5)
#define CAR_STATE_BUFFER_SIZE (6)
#define MSG_BUFFER_SIZE  (50)  // null까지 포함한 길이
char msg[MSG_BUFFER_SIZE];
char strSonar[SONAR_BUFFER_SIZE]; 
char strcarState[CAR_STATE_BUFFER_SIZE];

const int cnt = sizeof(enable)/sizeof(enable[0]);
String dir;
const int echo = 16;
const int trig = 17;
float duration,  distance;
int sonarAlt = 0;

const char* ssid = "IT";
const char* password = "@Polytech";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;



void setup() {
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  Serial.begin(115200);
  timer_init();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // callback 함수 연결
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
  
  // 연결이 끊기면 재연결
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sonar();

    if (count0 > 0) {
       // Comment out enter / exit to deactivate the critical section 
       portENTER_CRITICAL(&timerMux); // 카운트 진입 
       count0--;                       // 트리거 시간 감소
       portEXIT_CRITICAL(&timerMux);  // 카운트 종료

       String sonarData = "{\"distance\":\""+String(strSonar)+"\", \"state\":\""+String(strcarState) +"\"}";
       sonarData.toCharArray(msg, sonarData.length()+1); //string -> char[], char* 길이는 순수한 길이, 스트링은 NULL 포함
       Serial.print("Publish message: ");
       Serial.println(msg);
       client.publish("MP/Polytech/car1", msg);

       
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

  
  snprintf(strSonar, SONAR_BUFFER_SIZE, "%d", sonarAlt);
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
  if ((char)payload[0] == '1') {
    Serial.println("1받음");
    digitalWrite(BUILTIN_LED, HIGH);
    

    
  } else {
    Serial.println("0받음");
    digitalWrite(BUILTIN_LED, LOW);
    

    
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
