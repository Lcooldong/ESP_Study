#include <esp_now.h>
#include <WiFi.h>
#include <Keypad.h>


////////////////////////
////////KeyPad//////////
////////////////////////
const uint8_t ROWS = 4;
const uint8_t COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

//ESP32 Dev module 38Pin
byte rowPins[ROWS] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 33, 32}; //connect to the column pinouts of the keypad

//Create an object of keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

///////////////////////


uint8_t wemos_d1mini[] = {0x08, 0x3A, 0xF2, 0x7D, 0x48, 0xE0};
uint8_t esp32_38pin[] = {0x08, 0x3A, 0xF2, 0xAA, 0x0C, 0xEC};
uint8_t wemos_lite[] = {0x78, 0xE3, 0x6D, 0x19, 0x2F, 0x44};

String success;
uint8_t incomingRGB[3];
uint8_t RGB_PIN[3] = {19, 18, 17};

typedef struct _struct_message {
    uint8_t RGB[3];
    char Password[32];
    uint8_t Password_Length;
} struct_message;

struct_message incomingReadings;
struct_message myState;
int pw_length = 0;
char pw_buffer[32];
char send_pw_buffer[32];
String pw_str;
uint8_t pw_cnt = 0;
uint8_t send_pw_cnt = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
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
  memcpy(peerInfo.peer_addr, wemos_lite, 6); // 6-> address length
  peerInfo.channel = 1;
  peerInfo.encrypt = false; // 암호화 ID/PW

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }else{
    Serial.println("Succeed to add peer");  
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  for(int i=0; i<sizeof(RGB_PIN); i++)
  {
    //pinMode(RGB_PIN[i], OUTPUT);  
    ledcAttachPin(RGB_PIN[i], i);
    ledcSetup(i, 5000, 8);
  }


  strcpy(myState.Password, "0000");
  pw_str = myState.Password;  // char* -> string

  Serial.print("String : ");
  Serial.print(pw_str);
  pw_length = pw_str.length(); 
  Serial.print("| char PW[32] : ");
  Serial.print(myState.Password);
  Serial.print("| LENGTH : ");
  Serial.println(pw_length);

  memset(send_pw_buffer, '0', sizeof(pw_length)); // char 숫자 0 은 NULL
  Serial.print("| send_Buffer : ");
  for(int i=0; i< pw_length; i++)
  {
    Serial.print(send_pw_buffer[i]);
  }
  
  Serial.println("\n==========Setup============");
}

void loop() {

  if(Serial.available()){
    Serial.read();
  }

  char key = keypad.getKey();
  if(key){
    Serial.print("Key Pressed : ");
    Serial.println(key);
    if((key >='0') && (key <='9'))
    {
      if(pw_length > pw_cnt)
      {
        pw_buffer[pw_cnt] = key;
        Serial.print("CNT : ");
        Serial.println(pw_cnt);
        pw_cnt++;
        if(pw_length == pw_cnt) pw_cnt = 0;
      }
 
      Serial.print("Pressed pw : ");
      for(int i=pw_cnt; i < pw_length; i++)
      {
        Serial.print(pw_buffer[i]);
        
        
      }
      for(int j=0; j < pw_cnt; j++)
      {
        Serial.print(pw_buffer[j]);
       
      }
      Serial.println();
      
      send_pw_cnt = pw_cnt;
      for(int k=0; k< pw_length; k++ )
      {
        send_pw_buffer[k] = pw_buffer[send_pw_cnt]; 
        send_pw_cnt++;
        if(send_pw_cnt == pw_length) send_pw_cnt = 0;
      }
          
    }
    else if(key == '*')
    {
      Serial.println("=====comfirm to open======");
      Serial.print("Buffer Array : ");
      for(int i=0; i < pw_length; i++)
      {
        myState.Password[i] = send_pw_buffer[i];
        Serial.print(send_pw_buffer[i]);
      }
      Serial.println();
      //myState.Password = send_pw_buffer;
      myState.Password_Length = pw_length;
      Serial.print("struct PW : ");
      Serial.println(myState.Password);
      Serial.print("struct PW_Length : ");
      Serial.println(myState.Password_Length);
      
      myState.RGB[0] = 0;
      myState.RGB[1] = 0;
      myState.RGB[2] = 0;
    
      Serial.println();
      Serial.print("송신 -> Red : ");
      Serial.println(myState.RGB[0]);
      Serial.print("Green : ");
      Serial.println(myState.RGB[1]);
      Serial.print("Blue : ");
      Serial.println(myState.RGB[2]);

      //esp_err_t result = esp_now_send(wemos_lite, (uint8_t *) &myState, sizeof(myState));
      esp_err_t result = esp_now_send(wemos_lite, (uint8_t *) &myState, sizeof(myState));

      if (result == ESP_OK) 
      {
        Serial.println("Sent Password with success");
      }
      else 
      {
        Serial.println("Error sending the Password");
      }
      
    }
    else
    {
      pw_cnt = 0;
      for(int i=0; i <pw_length; i++)
      {
        pw_buffer[i] = '#';
        send_pw_buffer[i] = '#';
      }
      Serial.print("Reset buffer : ");
      for(int j=0; j < pw_length; j++)
      {
        Serial.print(pw_buffer[j]);
      }
      Serial.println();
    }
  }
}


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
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
  for(int i = 0; i<sizeof(incomingRGB); i++)
  {
      incomingRGB[i] = incomingReadings.RGB[i];
  }

  Serial.print("수신 -> Red : ");
  Serial.println(incomingRGB[0]);
  Serial.print("Green : ");
  Serial.println(incomingRGB[1]);
  Serial.print("Blue : ");
  Serial.println(incomingRGB[2]);
  for(int i=0; i< sizeof(RGB_PIN); i++)
  {
    ledcWrite(i, incomingRGB[i]);
  }


  Serial.print("Received PW : ");
  Serial.println(incomingReadings.Password);
  
}
