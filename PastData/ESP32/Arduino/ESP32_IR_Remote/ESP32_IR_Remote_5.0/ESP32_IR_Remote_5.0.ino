#include <IRremote.h>
//#include "PinDefinitionsAndMore.h"
#define DECODE_NEC 1  // 디코딩 방식
#define timeSeconds 0.1 // 초
#define TIME_BUFFER_SIZE 30
#define RAW_BUFFER_SIZE 11
#define SAVE_DATA_SIZE 5

const int IR_RECEIVE_PIN = 23;  // 데이터 핀
const int LED_PIN = 27;
const int BTN = 34;
const int RGB[3] = {16, 17, 18};
int colorState[3] = {0,};
int flag = 0;
int state = LOW;

unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;

char strTime[TIME_BUFFER_SIZE];
char hexbuf[RAW_BUFFER_SIZE];
uint16_t* storage[SAVE_DATA_SIZE];


uint16_t sAddress = 0x0000;
uint8_t sCommand  = 0x00;
uint8_t sRepeats  = 0;


// Storage for the recorded code IR저장구조체
struct storedIRDataStruct {
    IRData receivedIRData;
    // extensions for sendRaw
    uint8_t rawCode[RAW_BUFFER_LENGTH]; // The durations if raw
    uint8_t rawCodeLength; // The length of the code
} sStoredIRData;



void setup() {
    Serial.begin(115200);
    
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // 데이터 핀, 수신 확인용 LED<- 잘모르겠음
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver, enable feedback LED, take LED feedback pin from the internal boards definition
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin
    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
    pinMode(LED_PIN, OUTPUT);
    for(int i = 0; i<3; i++){
      ledcSetup(i, 5000, 8);
      ledcAttachPin(RGB[i], i);
      ledcWrite(i, 0);
    }
    pinMode(BTN, INPUT_PULLUP);
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
        //Serial.println();
        IrReceiver.resume(); // Enable receiving of the next value
        
        /*
         * Finally check the received data and perform actions according to the received commands
         */
        storage[0] = &IrReceiver.decodedIRData.command;  // 포인터는 주소, 구조체command : uint16_t
        Serial.print("storage[0] : 0x");
        Serial.println(*storage[0], HEX);  // 역참조, 주소의 값
        //sendData();
        printRawData();
        executeCommand();
    }
    timeInterval();
}

//void storeCode(IRData *aIRReceivedData) {
//    if (aIRReceivedData->flags & IRDATA_FLAGS_IS_REPEAT) {
//        Serial.println(F("Ignore repeat"));
//        return;
//    }
//    if (aIRReceivedData->flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
//        Serial.println(F("Ignore autorepeat"));
//        return;
//    }
//    if (aIRReceivedData->flags & IRDATA_FLAGS_PARITY_FAILED) {
//        Serial.println(F("Ignore parity error"));
//        return;
//    }
//    /*
//     * Copy decoded data
//     */
//    sStoredIRData.receivedIRData = *aIRReceivedData;
//
//    if (sStoredIRData.receivedIRData.protocol == UNKNOWN) {
//        Serial.print(F("Received unknown code saving "));
//        Serial.print(IrReceiver.results.rawlen - 1);
//        Serial.println(F(" TickCounts as raw "));
//        sStoredIRData.rawCodeLength = IrReceiver.results.rawlen - 1;
//        IrReceiver.compensateAndStoreIRResultInArray(sStoredIRData.rawCode);
//    } else {
//        IrReceiver.printIRResultShort(&Serial);
//        sStoredIRData.receivedIRData.flags = 0; // clear flags -esp. repeat- for later sending
//        Serial.println();
//    }
//}

void executeCommand(){
  uint8_t myCommand = IrReceiver.decodedIRData.command;
  switch(myCommand){
    case 0x45: LED_ON_OFF(); break;
    case 0x44: RGB_ON_OFF(0, 255); break;
    case 0x40: RGB_ON_OFF(1, 255); break;
    case 0x43: RGB_ON_OFF(2, 255); break;
  }
}

void sendData(){
    sAddress = 0x0001;
    sCommand = 0x45;
    sRepeats = 2;
  
    Serial.println();
    Serial.print(F("Send now: address=0x"));
    Serial.print(sAddress, HEX);
    Serial.print(F(" command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(" repeats="));
    Serial.print(sRepeats);
    Serial.println();

    Serial.println(F("Send NEC with 16 bit address"));
    Serial.flush();

    // Results for the first loop to: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
    IrSender.sendNEC(sAddress, sCommand, sRepeats);

    /*
     * If you cannot avoid to send a raw value directly like e.g. 0xCB340102 you must use sendNECRaw()
     */
//    Serial.println(F("Send NECRaw 0xCB340102"));
//    IrSender.sendNECRaw(0xCB340102, sRepeats);

    delay(1000);
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

void RGB_ON_OFF(int color, int brightness){
    startTimer = true;
    lastTrigger = millis();   // 신호가 들어온 시간
    Serial.print("작동 시간 : ");
    Serial.println(lastTrigger);
    if(flag == 0){
      flag = 1;           //flag = 1 일 때는 추가 신호 안받음
      colorState[color] = !colorState[color];
      delay(100);
      Serial.print("state : ");
      Serial.println(colorState[color]);
      ledcWrite(color, brightness*colorState[color]);
      Serial.println("LED state Changed");
  }
}
