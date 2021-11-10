#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#define BUILTIN_LED 9

const char* ssid = "LGU+_POLY";
const char* password = "@Polytech";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
const int rPin = 4;
int rVal = 0;


void setup() {
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if(!client.connected()){
    reconnect();  
  }
  client.loop();

  int rVal = analogRead(rPin);
  rVal = map(rVal, 0, 4095, 0, 200);
  //Serial.println(rVal);
  publishData();
}
