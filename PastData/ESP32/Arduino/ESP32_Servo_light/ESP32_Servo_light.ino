#include <WiFiManager.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32_Servo.h>

// 서보모터 GND 와 보드 GND 연결
// 외부전원 사용 필수


#define MSG_BUFFER_SIZE  (50)  // null까지 포함한 길이
char msg[MSG_BUFFER_SIZE];

volatile int count0;
hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

static const int servoPin = 27;
static const int ledPin = 15;

const char* ssid = "2KNG";
const char* password = "77120094";
const char* mqtt_server = "broker.mqtt-dashboard.com";
int degree = 0;


WiFiClient espClient;
PubSubClient client(espClient);

Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin);
    
//    ledcSetup(1, 5000, 8);     // channel, freq, resolution
//    ledcAttachPin(ledPin, 1);  // Pin, channel
//    ledcWrite(1, 0);           // channel, value

    WiFi.mode(WIFI_STA);
    WiFiManager wm;

    wm.resetSettings();
    bool res;
    res = wm.autoConnect("Light_Switch");

    if(!res){
      Serial.println("Failed to connect");  
    }else{
      Serial.println("connected");
      client.setServer(mqtt_server, 1883);
      client.setCallback(callback);  
    }
    timer_init();
    servo1.write(0);
}

void loop() {
    
    if(!client.connected()){
      reconnect(); 
    }
    client.loop();

  if (count0 > 0) {
     // Comment out enter / exit to deactivate the critical section 
     portENTER_CRITICAL(&timerMux);
     count0--;                       
     portEXIT_CRITICAL(&timerMux);
     String degreeData = "{\"degree\":\""+String(degree)+"\"}";
     degreeData.toCharArray(msg, degreeData.length()+1); //string -> char[], char* 길이는 순수한 길이, 스트링은 NULL 포함
     Serial.print("Publish message: ");
     Serial.println(msg);
     client.publish("Polytech/312/LIGHT", msg);
  }

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
    case '1':
      degree = 0;
      servo1.write(degree);
      Serial.println("1 received");
      break;
    case '2':
      degree = 180;
      servo1.write(degree);
      Serial.println("2 received");
      break;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-LIGHT";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("Polytech/312/LIGHT", "reconnected");    // 재연결 브로커로 송신(publish, payload)
      // ... and resubscribe
      client.subscribe("Polytech/312/server1");  // 재연결 구독(subscribe)->브로커에서 수신(payload 받아옴)
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

void timer_init(){
   // Configure the Prescaler at 80 the quarter of the ESP32 is cadence at 80Mhz
   // 80000000 / 80 = 1000000 tics / seconds  T: 0.000001s, 1초에 1,000,000 찍음
   timer0 = timerBegin(0, 80, true);              
   timerAttachInterrupt(timer0, &onTimer0, true);


       
    
   // Sets an alarm to sound every second
   //  number/1,000,000 -> 시간(s)   또는  원하는 시간/0.000001 -> 타이머 인터럽트가 발생되는 Count횟수
   timerAlarmWrite(timer0, 1000000, true);
   timerAlarmEnable(timer0);

}

void setup_wifi(){  //여기서는 사용안함
  
  delay(10);  
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
