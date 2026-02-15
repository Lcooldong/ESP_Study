#include "OTA.h"
#include "Neopixel.h"
#include <AsyncElegantOTA.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
 
HardwareSerial mySerial(2);

uint8_t wemos_d1mini[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
uint8_t esp32_38pin[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};
uint8_t wemos_lite[] = {0x78, 0xE3, 0x6D, 0x19, 0x2F, 0x44};
uint8_t esp_module_0[] = {0x58, 0xBF, 0x25, 0x17, 0x6D, 0x28};

String success;
uint8_t incomingRGB[3];

#pragma pack(push, 1)
typedef struct packet_
{
    uint8_t led_number; // 시작 사인 : 1
    uint8_t RED;  // 데이터 타입 : 1
    uint8_t GREEN;
    uint8_t BLUE;
    uint8_t brightness;
    uint8_t style;            // 데이터 : 4 
    uint8_t wait;
    uint8_t checksum;  // 체크섬 : 2
}PACKET;
#pragma pack(pop)

typedef enum {
  oneColor = 1,
  CHASE,
  RAINBOW
}STYLE_Typedef;

STYLE_Typedef _style;

PACKET serial_data = {0, };
PACKET incomingReadings;

int neopixel_Flag = 0;

unsigned long RESET_TIME = 1000*60*60*24; // 1 day
volatile int count0;
volatile int count1; 
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

bool FORMAT_SPIFFS_IF_FAILED = true;    // 마운트 실패시 SPIFFS 파일 시스템 포맷

esp_now_peer_info_t peerInfo;
//void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
//void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
AsyncWebServer server(80);

const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void setup() {
  
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 26, 27);
  mySerial.println("connected");
  init_Neopixel(50);
  initWiFi();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Connection Test -> /update to upload bin file");
  });
  AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
  server.begin();
  Serial.println("HTTP server started");
  
  if (esp_now_init() == ESP_OK) {
    // 0(ESP_OK) 일 때 시작
    Serial.println("ESP-NOW initialized");
  }else{
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  }

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  //memset(&peerInfo, 0, sizeof(peerInfo));
  
  memcpy(peerInfo.peer_addr, wemos_lite, 6);
  peerInfo.channel = 13;  
  peerInfo.encrypt = false; // 암호화 ID/PW
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }else{
    Serial.println("Succeed to add peer");
  }


//  add_peer(wemos_lite, 1);
//  add_peer(esp32_38pin, 2);
//  add_peer(wemos_d1mini, 1);
  //add_peer(esp_module_0, 4);

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.write("ESP_NOW_CONNECTED");
  Serial.println();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  
  TelnetStream.begin(); // wifi 보다 아래에 위치해야함
  timer_init();
  
}


void loop() {
  //reconnectWiFi();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  
  if(Serial.available())
  {  
    char ch = Serial.read();
    if(ch == 's')
    {    
      serial_data.led_number = 1;
      serial_data.RED = 255;
      serial_data.GREEN = 0;
      serial_data.BLUE = 123;
      serial_data.brightness = 50;
      serial_data.style = 1;
      serial_data.wait = 50;
      serial_data.checksum = 0;
      esp_err_t result = esp_now_send(wemos_lite, (uint8_t *) &serial_data, sizeof(serial_data));

      if (result == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
    } 
      
      // packet 사이즈만큼 읽어옴
//      Serial.readBytes((char*)&serial_data, sizeof(serial_data));
//      serial_data.checksum += 1;
//      neopixel_Flag = 1; 
//      Serial.write((char*)&serial_data, sizeof(serial_data));
//      pickOneLED(0, serial_data.RED, serial_data.GREEN, serial_data.BLUE, serial_data.brightness, serial_data.wait);
      StaticJsonDocument<256> doc;

      doc["led_number"] = serial_data.led_number;
      doc["RED"] = serial_data.RED;
      doc["GREEN"] = serial_data.GREEN;
      doc["BLUE"] = serial_data.BLUE;
      doc["brightness"] = serial_data.brightness;
      doc["style"] = serial_data.style;
      doc["wait"] = serial_data.wait;
      doc["checksum"] = serial_data.checksum;
   
      
//      JsonArray repeater = doc.createNestedArray("repeater");
//      for(int i =0;i<sensor.count;i++){ //중계된 만큼 count 증가
//        repeater.add(sensor.repeater[i]); // 만든 배열에 값 전달
//      }
//      doc["count"] = sensor.count;
      String output;
      serializeJson(doc, output);
    
      Serial.println(output);
      client.publish("PC_MQTT/LED_CONTROL", output.c_str());

       
      delay(50);   
  }


  if(serial_data.led_number == 255) 
  {
      server.end();
      pickOneLED(0, 255, 255, 255, 50, 50);
      print_output("Reset WiFi");
      changeWiFi();
      server.begin();
      ESP.restart();
  }

  
  if( neopixel_Flag == 1 ){
    
    neopixel_Flag = 0;  
    String string_style = "";
    
    switch(serial_data.style)
    {
      case oneColor:
        string_style = "oneColor";
        print_output(string_style);
        pickOneLED(0, serial_data.RED, serial_data.GREEN, serial_data.BLUE, serial_data.brightness, serial_data.wait);
        print_output(String(serial_data.RED));
        print_output(String(serial_data.GREEN));
        print_output(String(serial_data.BLUE));
        break;
      case CHASE:
        string_style = "chase";
        print_output(string_style);
        break;         
    }

    switch(serial_data.led_number)
    {
      case 0:
        esp_now_send(wemos_lite, (uint8_t *) &serial_data, sizeof(serial_data));
        print_output("LED : 0");
        break;
      case 1:
        esp_now_send(wemos_lite, (uint8_t *) &serial_data, sizeof(serial_data));
        print_output("LED : 1");
        break;
      case 2:
        esp_now_send(wemos_lite, (uint8_t *) &serial_data, sizeof(serial_data));
        print_output("LED : 2");
        break;
      case 3:
        esp_now_send(wemos_lite, (uint8_t *) &serial_data, sizeof(serial_data));
        print_output("LED : 3");
        break;
      case 4:
        esp_now_send(wemos_lite, (uint8_t *) &serial_data, sizeof(serial_data));
        print_output("LED : 4");
        break;
     }
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

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.println("get one");
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    Serial.println("not one");
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}


void print_output(String _string){
  TelnetStream.println(_string);
  mySerial.println(_string);
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
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void add_peer(uint8_t peer_name[], uint8_t _channel){
  /// register third peer
  memcpy(peerInfo.peer_addr, peer_name, 6);
  peerInfo.channel = _channel;  
  peerInfo.encrypt = false; // 암호화 ID/PW
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }else{
    Serial.println("Succeed to add peer");
  }
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  print_output("\r\nLast Packet Send Status:  ");
  print_output(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.print("\r\nLast Packet Send Status:  ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // 받을 구조체, 버퍼(받은 데이터), 구조체 크기
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  Serial.print("Bytes received: ");
  Serial.println(len);

  incomingRGB[0] = incomingReadings.RED;
  incomingRGB[1] = incomingReadings.GREEN;
  incomingRGB[2] = incomingReadings.BLUE;
  

  Serial.print("수신 -> Red : ");
  Serial.println(incomingRGB[0]);
  Serial.print("Green : ");
  Serial.println(incomingRGB[1]);
  Serial.print("Blue : ");
  Serial.println(incomingRGB[2]);
 
}



void IRAM_ATTR onTimer0() {
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
   timerAlarmWrite(timer0, 2000000, true);
   timerAlarmWrite(timer1, 30000000, true); 
   
   timerAlarmEnable(timer0);
   timerAlarmEnable(timer1);
}
