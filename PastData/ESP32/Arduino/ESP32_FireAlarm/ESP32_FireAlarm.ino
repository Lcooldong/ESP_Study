#define BUILTIN_LED 2
#define LED 5
#define DHTTYPE DHT11
#define DHTPIN 15

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

DHT dht(DHTPIN, DHTTYPE);


// Update these with values suitable for your network.

const char* ssid = "214ho";
const char* password = "12345678";
//const char* ssid = "LDH";
//const char* password = "ehdgml43";
//const char* ssid = "HCN_9E91";
//const char* password = "7263FB9E90";
//const char* ssid = "iptime";
//const char* password = "";  //비번 없이도 가능
//const char* ssid = "ㄱㄱㅈ";
//const char* password = "atelsit84"; 
//const char* ssid = "AndroidHotspot2221";
//const char* password = "01086122221";  
//const char* ssid = "LGU+_POLY";
//const char* password = "@Polytech";  
//const char* ssid = "IT";
//const char* password = "@Polytech";  
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
//#define MSG_BUFFER_SIZE  (28)
char msg[MSG_BUFFER_SIZE];
char strhumi[5];
char strtemp[5];
int value = 0;

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
      client.publish("FireAlarm/Polytech/A1", "reconnected");    // 재연결 브로커로 송신(publish, payload)
      // ... and resubscribe
      client.subscribe("FireAlarm/Polytech/Server");  // 재연결 구독(subscribe)->브로커에서 수신(payload 받아옴)
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // callback 함수 연결
  dht.begin();
}

void loop() {

  // 연결이 끊기면 재연결
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();// 습도를 측정합니다.
  float t = dht.readTemperature();// 온도를 측정합니다.
  float f = dht.readTemperature(true);// 화씨 온도를 측정합니다.

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    //snprintf (msg, MSG_BUFFER_SIZE, "%.1f, %.1f", h, t);
    snprintf(strhumi, 5, "%.1f", h);
    snprintf(strtemp, 5, "%.1f", t);
    
    String dhtData = "{\"humidity\":\""+String(strhumi)+"\", \"temperature\":\""+String(strtemp) +"\"}";
    dhtData.toCharArray(msg, dhtData.length()+1); //string -> char[], char*
    //Serial.println(dhtData.length()); //41
    
    //String dhtData = "{\"humidity\":\""+String(h)+"\", \"temperature\":\""+String(t) +"\"}";
    //String dhtData = "{\"humidity\":\""+String(h)+"\", \"temperature\":\""+String(t) +"\"}";
    //snprintf (msg, MSG_BUFFER_SIZE, "{\"humidity\":\""+String(%.1f)+"\", \"temperature\":\""+String(%.1f) +"\"}", h, t);
    Serial.print("Publish message: ");
    //Serial.println(dhtData);
    Serial.println(msg);
    client.publish("FireAlarm/Polytech/A1", msg);  // 브로커로 송신(publish, payload)
    //client.publish("FireAlarm/Polytech/A1", dhtData);
  }
}
