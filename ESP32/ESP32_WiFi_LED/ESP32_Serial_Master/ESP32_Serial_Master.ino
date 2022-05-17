#include "OTA.h"
#include <HardwareSerial.h>
#define EEPROM_SIZE 128

#pragma pack(push, 1)
typedef struct packet_
{
    uint8_t led_number; // 시작 사인 : 1
    uint8_t RED;  // 데이터 타입 : 1
    uint8_t GREEN;
    uint8_t BLUE;
    uint8_t style;            // 데이터 : 4
    uint8_t brightness;
    uint8_t checksum;  // 체크섬 : 2
}PACKET;
#pragma pack(pop)

typedef enum {
  oneColor = 1,
  CHASE,
  RAINBOW
}STYLE_Typedef;


HardwareSerial mySerial(2);
PACKET _data = {0, };
STYLE_Typedef _style;

char packet_buffer[128];

int cnt = 0;
int neopixel_Flag = 0;


void setup() {
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 26, 27);
  mySerial.println("connected");
  init_Neopixel(50);
  initOLED();
  EEPROM.begin(EEPROM_SIZE);
  setupOTA("ESP_LED");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.write("WIFI_CONNECTED");
}

void loop() {
  reconnectWiFi();
  ArduinoOTA.handle();
  //pickOneLED(0, strip.Color(255,   0,   0), 50);
//  rainbow(10);

  if(Serial.available())
  {  
      // packet 사이즈만큼 읽어옴
      Serial.readBytes((char*)&_data, sizeof(_data));
       _data.checksum += 1;
      Serial.write((char*)&_data, sizeof(_data));
      if( _data.checksum == 1){
        neopixel_Flag = 1;  
      }
      String string_style = "";
      switch(_data.style)
      {
        case oneColor:
          string_style = "oneColor";
          poutput(string_style);
          pickOneLED(0, _data.RED, _data.GREEN, _data.BLUE, _data.brightness);
          break;
        case CHASE:
          string_style = "chase";
          poutput(string_style);
          break;  
          
      }
      
      const char* format = "%d : [%d, %d, %d] style: %s, %d, %d";
      sprintf(packet_buffer, format, _data.led_number,
                                     _data.RED,
                                     _data.GREEN,
                                     _data.BLUE,
                                     string_style,
                                     _data.brightness,
                                     _data.checksum );
                                     
      poutput(packet_buffer);
      delay(100);
  }
 

}

void poutput(String _string){
  TelnetStream.println(_string);
  mySerial.println(_string);
}
