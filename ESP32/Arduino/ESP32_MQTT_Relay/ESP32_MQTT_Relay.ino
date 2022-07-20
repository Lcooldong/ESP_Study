#include <WiFiManager.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "FileSystem.h"

#define LED_PIN 2
const int relay_1 = 15;

#define MSG_BUFFER_SIZE  (50)  // null까지 포함한 길이
char msg[MSG_BUFFER_SIZE];

volatile int count0;
volatile int count1;
int totalInterrupts;
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

bool FORMAT_SPIFFS_IF_FAILED = true;
int WiFiManager_flag = 0;
int connection_flag = 0;
unsigned long connectingTime = 0;
unsigned long connecting_interval = 10000;

//const char* ssid = "409";
//const char* password = "polybot409";
const char* mqtt_server = "broker.mqtt-dashboard.com";


char ssid[32]          = "YOUR_SSID";
char pass[32]          = "YOUR_PASS";
char TEMP_SSID[32] = {0,};
char TEMP_PASS[32] = {0, };

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
//  for(int i = 0; i < cnt; i++){
//    ledcSetup(i, 5000, 8);
//    ledcAttachPin(enable[i], i);
//    ledcWrite(i, 0);
//  }
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA); // WiFiManager 사용시 AP 모드 필수
  WiFiManager wm;
  
  

  init_SPIFFS(FORMAT_SPIFFS_IF_FAILED);
  listDir("/");
  Serial.println("--------Saved Data--------");
  if (SPIFFS.exists("/config.txt")) loadConfig();
  else saveConfig();
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  //readFile("/config.txt");
  Serial.println("--------------------------");


  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    
    Serial.print('.');
    delay(1000);   
    connectingTime = millis();
    if(connectingTime >= connecting_interval)
    {
      Serial.println();
      Serial.println("Not Connected -> Open WiFi Manager");
      
      WiFiManager_flag = 1;
      break;
    }
  }
  if(WiFiManager_flag == 1)
  {
    wm.resetSettings();
    bool res;
//    sprintf(WiFiManager_Name, "RemoteRelay_%d", device_id);
//    res = wm.autoConnect(WiFiManager_Name);
    res = wm.autoConnect("RemoteRelay");
    
    if(!res) 
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else 
    {
        String SSID_NAME = WiFiManager().getWiFiSSID();
        String PW_NAME = WiFiManager().getWiFiPass();
        
        Serial.print("SSID : ");
        Serial.println(SSID_NAME);
      
        Serial.print("PASSWORD : ");
        Serial.println(PW_NAME);
     
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    
        
        SSID_NAME.toCharArray(TEMP_SSID, sizeof(TEMP_SSID));
        PW_NAME.toCharArray(TEMP_PASS, sizeof(TEMP_PASS));

        for(int i=0; i< sizeof(ssid); i++){
          ssid[i] = TEMP_SSID[i];
          pass[i] = TEMP_PASS[i];
        }
        saveConfig();
        ESP.restart();
    }
  }
  else 
  {
    Serial.print("connected ->");
    Serial.println(WiFi.localIP());
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback); // callback 함수 연결
    client.publish("Polytech/Mailbox/relay1", "connected");
    client.subscribe("Polytech/Mailbox/server1");
  }
  Serial.println("----------Result----------");
  Serial.print("SSID : ");
  Serial.println(ssid);
  Serial.print("PASS : ");
  Serial.println(pass);
  Serial.println("--------------------------");

  
  pinMode(LED_PIN, OUTPUT);
  pinMode(relay_1, OUTPUT);
  timer_init();
  //setup_wifi();
  
}



void loop() {

  // 연결이 끊기면 재연결
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  
  if (count0 > 0) {
     // Comment out enter / exit to deactivate the critical section 
     portENTER_CRITICAL(&timerMux);
     count0--;
     portEXIT_CRITICAL(&timerMux);
  }

  if (count1 > 0) {
       // Comment out enter / exit to deactivate the critical section 
       portENTER_CRITICAL(&timerMux); // 카운트 시작 
       count1--;                       // 트리거 시간 감소
       portEXIT_CRITICAL(&timerMux);  // 카운트 종료
  
//       String batteryData = "{\"voltage\":\""+String()+"\"}";
//       batteryData.toCharArray(msg, batteryData.length()+1); //string -> char[], char* 길이는 순수한 길이, 스트링은 NULL 포함
//       Serial.print("Publish message: ");
//       Serial.println(msg);
//       client.publish("Polytech/Mailbox/relay1", msg);
       client.publish("Polytech/Mailbox/relay1", "HEART_BEAT");
       
       totalInterrupts++; // 전체 인터럽트 횟수
//       Serial.print("totalInterrupts");
//       Serial.println(totalInterrupts);
       if ( totalInterrupts%2 == 0) {   // 짝수면 켜짐, 홀수면 꺼짐
         // Lights up the LED if the counter is even 
         digitalWrite(LED_PIN, HIGH);
       } else {
         // Then swith off
         digitalWrite(LED_PIN, LOW);
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
  WiFi.begin(ssid, pass);

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
    case '1': digitalWrite(relay_1, LOW); break;
    case '2': digitalWrite(relay_1, HIGH); break;
    case '3':  break;
    case '4':  break;
    case '5':  break;
    case '6':  break;
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
      client.publish("Polytech/Mailbox/relay1", "reconnected");    // 재연결 브로커로 송신(publish, payload)
      // ... and resubscribe
      client.subscribe("Polytech/Mailbox/server1");  // 재연결 구독(subscribe)->브로커에서 수신(payload 받아옴)
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
