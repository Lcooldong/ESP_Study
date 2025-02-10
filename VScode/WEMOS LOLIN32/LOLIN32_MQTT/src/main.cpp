#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

//#define DEBUG

const char* ssid = "KT_GiGA_2G_Wave2_1205";
const char* password = "8ec4hkx000";
const char* mqtt_server = "172.30.1.36";
const char* client_name = "ESP32_LOLIN32";
const char* mqtt_username = "mqttmanager";
const char* mqtt_password = "ehdgml43!";

const char* testCmndLedTopic = "cmnd/lolin/leds";
const char* testStateLedTopic = "stat/lolin/leds";
const char* testInTopic = "inTopic";
const char* testOutTopic = "outTopic";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
bool isTestComnd = false;


void setup_wifi();
boolean reconnect();
void callback(char* topic, byte* payload, unsigned int length);



void setup() {
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  if(client.connect(client_name, mqtt_username, mqtt_password))
  {
    Serial.println("MQTT Connected");
    client.publish(testOutTopic, "hello world");
  }
  else
  {
    Serial.println("MQTT Failed");
  }
  client.setCallback(callback);
  client.subscribe(testInTopic);
  client.subscribe("inTopic1");
  client.subscribe(testCmndLedTopic);
  client.subscribe(testStateLedTopic);

  digitalWrite(BUILTIN_LED, LOW); // LOW 가 ON

}

void loop() {
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
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


boolean reconnect() {
  // Loop until we're reconnected

#ifdef DEBUG
    Serial.print("Attempting MQTT connection...");
#endif
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(testOutTopic, "hello world");
      // ... and resubscribe
      client.subscribe(testInTopic);

      return true;
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      return false;
    }  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // if(!strcmp(topic, testInTopic))
  // {
  //   Serial.println("receive inTopic");

  //   // // Switch on the LED if an 1 was received as first character
  //   // if ((char)payload[0] == '1') {
  //   //   digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
  //   //   // but actually the LED is on; this is because
  //   //   // it is active low on the ESP-01)
  //   // } else {
  //   //   digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  //   // }

  // }
  // else
  // {
  //   Serial.println("receive otherTopic");
  // }

  if(!strcmp(topic, testCmndLedTopic))
  {
    Serial.println("receive Command");
    isTestComnd = true;
  }
  else
  {

  }

  if(isTestComnd)
  {
    if ((char)payload[0] == '0')
    {
      digitalWrite(BUILTIN_LED, HIGH);
      client.publish(testStateLedTopic, "OFF");
      Serial.println("LED OFF");
    } 
    else 
    {
      digitalWrite(BUILTIN_LED, LOW);
      client.publish(testStateLedTopic, "ON");
      Serial.println("LED ON");
    }

    isTestComnd = false;
  }
  
}