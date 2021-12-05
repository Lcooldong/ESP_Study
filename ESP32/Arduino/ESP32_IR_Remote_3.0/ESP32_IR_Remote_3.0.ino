#define DECODE_NEC 1  // 디코딩 방식
#include <IRremote.h>
#define timeSeconds 0.1 // 초
#define TIME_BUFFER_SIZE 30
#define RAW_BUFFER_SIZE 11

const int IR_RECEIVE_PIN = 23;  // 데이터 핀
const int LED_PIN = 27;

int flag = 0;
int state = LOW;

unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;

char strTime[TIME_BUFFER_SIZE];
char hexbuf[RAW_BUFFER_SIZE];

void setup() {
    Serial.begin(115200);
    
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // 데이터 핀, 수신 확인용 LED<- 잘모르겠음
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver, enable feedback LED, take LED feedback pin from the internal boards definition
    Serial.print(F("Ready to receive IR signals at pin "));
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void loop() {
    /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     */
     // 디코딩 된 정보 받으면
    if (IrReceiver.decode()) {
        Serial.println();
        // Print a short summary of received data1바이트 형식
        IrReceiver.printIRResultShort(&Serial); // 받은 데이터 시리얼에 표시
        // 알 수 없는 것이 들어오면 추가 정보 출력
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            // We have an unknown protocol, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
        Serial.println();
        IrReceiver.resume(); // Enable receiving of the next value
        
        /*
         * Finally check the received data and perform actions according to the received commands
         */
        printRawData();
        executeCommand();
    }
    timeInterval();
}

void executeCommand(){
  uint16_t myCommand = IrReceiver.decodedIRData.command;
  switch(myCommand){
    case 0x45: LED_ON_OFF(); break;
    case 0x46: break;
  }
}

void timeInterval(){
    now = millis(); // 현재 시간은 계속 측정 중 -> 신호 때 받은 시간과 비교
    if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
        Serial.print("현재 시간 : ");
        Serial.println(now);
        float interval = (float)(now-lastTrigger)/1000;
        snprintf(strTime, TIME_BUFFER_SIZE, "간격 : %.1f 초", interval);
        Serial.println(strTime);
        flag = 0;
        startTimer = false;
        Serial.println("Initializing");
    }
}

void printRawData(){
    Serial.println("=============================");
    Serial.print("RawData[DECIMAL] : ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData);
    
    snprintf(hexbuf, RAW_BUFFER_SIZE, "0x%08X", IrReceiver.decodedIRData.decodedRawData);
    Serial.print("RawData[HEX]     : ");
    Serial.println(hexbuf);
    Serial.println("=============================");
}

void LED_ON_OFF(){
    startTimer = true;
    lastTrigger = millis();   // 신호가 들어온 시간
    Serial.print("작동 시간 : ");
    Serial.println(lastTrigger);
    if(flag == 0){
      flag = 1;           //flag = 1 일 때는 추가 신호 안받음
      state = !state;
      delay(100);
      Serial.print("state : ");
      Serial.println(state);
      digitalWrite(LED_PIN, state);
      Serial.println("LED state Changed");
  }
}
