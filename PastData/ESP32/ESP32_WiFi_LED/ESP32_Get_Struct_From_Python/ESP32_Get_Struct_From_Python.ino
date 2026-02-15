#include <HardwareSerial.h>

HardwareSerial mySerial(2);

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

//또는 
//unsigned char packet[8];

// 데이터를 받을 버퍼 선언
PACKET _data = {0, };
void setup()
{
    Serial.begin(115200);
    mySerial.begin(115200, SERIAL_8N1, 26, 27);
    //mySerial.println("connected");  // 확인용

    //Serial.print(sizeof(_data));
    
}

void loop()
{
    // 시리얼에 읽을 데이터가 있다면
    if(Serial.available())
    {  
        // packet 사이즈만큼 읽어옴
        Serial.readBytes((char*)&_data, sizeof(_data));
        // 데이터 값에 + 1을 한 뒤에 다시 전송
        _data.checksum += 1;
        Serial.write((char*)&_data, sizeof(_data));
        mySerial.println(sizeof(_data));

        delay(100);
    }

}
