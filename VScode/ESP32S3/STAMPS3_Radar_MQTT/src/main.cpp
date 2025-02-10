#include <Arduino.h>
#include <FastLED.h>
#include <DFRobot_mmWave_Radar.h>
#include <PubSubClient.h>
#include <WiFi.h>



/// @brief PIN
const int PIN_BUTTON = 0;
const int PIN_LED    = 21;
const int NUM_LEDS   = 1;

/// @brief CONFIG
const char* ssid = "KT_GiGA_2G_Wave2_1205";
const char* password = "8ec4hkx000";
const char* mqtt_server = "172.30.1.36";
const char* client_name = "ESP32_STAMPS3";
const char* mqtt_username = "mqttmanager";
const char* mqtt_password = "ehdgml43!";

const char* cmndPresenceTopic = "cmnd/stampS3/radar";
const char* statePresenceTopic = "stat/stampS3/radar";

const char* detectState = "Detected";
const char* notExistState = "Not Exists";

const char* testInTopic = "inTopic";
const char* testOutTopic = "outTopic";


/// @brief Test Value
uint32_t lastMillis = 0;
int count = 0;
CRGB leds[NUM_LEDS];

/// @brief MQTT Value
#define MSG_BUFFER_SIZE	(32)
char msg[MSG_BUFFER_SIZE];
bool isComnd = false;
char tempMsg[32] = "";
char lastMsg[32] = "";

/// @brief filter
const int windowSize = 5;
int sensorData[windowSize];
int filteredData = 0;



/// @brief Object
WiFiClient espClient;
PubSubClient client(espClient);
HardwareSerial mySerial(1);
DFRobot_mmWave_Radar sensor(&mySerial);


/// @brief Function
void setup_wifi();
boolean reconnect();
void callback(char* topic, byte* payload, unsigned int length);
int getMedian(int data[], int size);

void setup()
{
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 1, 3);//RX,TX
  pinMode(PIN_BUTTON, INPUT);
  FastLED.addLeds<WS2812, PIN_LED, GRB>(leds, NUM_LEDS);

  delay(1000);
  
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  if(client.connect(client_name, mqtt_username, mqtt_password))
  {
    Serial.println("MQTT Connected");
    client.publish(statePresenceTopic, "start");
  }
  else
  {
    Serial.println("MQTT Failed");
  }
  client.setCallback(callback);
  client.subscribe(cmndPresenceTopic);
  client.subscribe(statePresenceTopic);


  sensor.factoryReset();    //Restore to the factory settings 
  //sensor.DetRangeCfg(0, 9);    //The detection range is as far as 9m
  sensor.DetRangeCfg(0, 3);
  sensor.OutputLatency(0, 0);

  leds[0] = CRGB(255, 0, 255);
  FastLED.show();
}

void loop()
{
  int val = sensor.readPresenceDetection();
  if (!client.connected()) 
  {
    reconnect();
  }
  else
  {
    
    
    memset(tempMsg, 0, sizeof(tempMsg));

    for (int i = 0; i < windowSize - 1; i++)
    {
      sensorData[i] = sensorData[i+1];
    }
    sensorData[windowSize - 1] = val;
    filteredData = getMedian(sensorData, windowSize);
  
    // if(val)
    if(filteredData)
    {
      leds[0] = CRGB(0, 0, 255);
      FastLED.show();
      strcpy(tempMsg, detectState);
      //client.publish(statePresenceTopic, detectState);
    }
    else
    {
      leds[0] = CRGB(255, 0, 0);
      FastLED.show();
      strcpy(tempMsg, notExistState);
      //client.publish(statePresenceTopic, notExistState);
    }
    Serial.println(val);  // Current Value
    //Serial.println(filteredData);
    
  
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= 1000 ) //&& tempMsg != 0
  {
    lastMillis = currentMillis;
    ++count;
    //Serial.printf("%d\r\n", count);
    if(strcmp(lastMsg, tempMsg))
    {
      client.publish(statePresenceTopic, tempMsg);
    }
    strcpy(lastMsg, tempMsg);
    //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", count);
    //Serial.print("Publish message: ");
    //Serial.println(msg);
    // client.publish("outTopic", msg);
  }
}





/// @brief FUNCTION LINE ------------------------------------------------------------------
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  IPAddress ip (172, 30, 1, 50);
  IPAddress gateway (172, 30, 1, 254);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
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
      client.publish(statePresenceTopic, "reconnect");
      // ... and resubscribe
      client.subscribe(cmndPresenceTopic);

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

  if(!strcmp(topic, cmndPresenceTopic))
  {
    Serial.println("receive Command");
    isComnd = true;
  }
  else
  {

  }

  if(isComnd)
  {
    if ((char)payload[0] == '0')
    {
      //digitalWrite(BUILTIN_LED, HIGH);
      client.publish(statePresenceTopic, "OFF");
      Serial.println("LED OFF");
    } 
    else 
    {
      //digitalWrite(BUILTIN_LED, LOW);
      client.publish(statePresenceTopic, "ON");
      Serial.println("LED ON");
    }

    isComnd = false;
  }
  
}

int getMedian(int data[], int size) {
  int temp[size];
  memcpy(temp, data, size * sizeof(int));  // Copy the data array to a temporary array
  // Sort the temporary array
  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (temp[i] > temp[j]) {
        int swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
      }
    }
  }
  // Return the median value
  return temp[size / 2];
}
