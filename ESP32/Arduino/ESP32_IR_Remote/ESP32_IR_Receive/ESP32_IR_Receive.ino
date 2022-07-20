// 참고 https://www.basic4mcu.com/bbs/board.php?bo_table=gac&wr_id=6598
/*
 * SimpleReceiver.cpp
 *
 * Demonstrates receiving NEC IR codes with IRrecv
 *
 *  Copyright (C) 2020-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.
 *
 *  MIT License
 */

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols are active.
 */
 
/*
#define DECODE_DENON        // Includes Sharp
#define DECODE_JVC
#define DECODE_KASEIKYO
#define DECODE_PANASONIC    // the same as DECODE_KASEIKYO
#define DECODE_LG
#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_SAMSUNG
#define DECODE_SONY
#define DECODE_RC5
#define DECODE_RC6

#define DECODE_BOSEWAVE
#define DECODE_LEGO_PF
#define DECODE_MAGIQUEST
#define DECODE_WHYNTER
*/

#define DECODE_NEC 1  // 디코딩 방식
#include <IRremote.h>

int IR_RECEIVE_PIN = 23;  // 데이터 핀
int LED_PIN = 27;
bool state = 0;
int LED_mode = 0;
uint32_t myRawdata= IrReceiver.decodedIRData.decodedRawData;
uint32_t myCommand = IrReceiver.decodedIRData.command;

void setup() {
    Serial.begin(115200);
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // 데이터 핀, 수신 확인용 LED<- 잘모르겠음
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver, enable feedback LED, take LED feedback pin from the internal boards definition
    pinMode(LED_PIN, OUTPUT);

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
}

void loop() {
    /*
     * Check if received data is available and if yes, try to decode it.
     * Decoded result is in the IrReceiver.decodedIRData structure.
     */
     // 디코딩 된 정보 받으면
    if (IrReceiver.decode()) {

        // Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial); // 받은 데이터 시리얼에 표시
        Serial.println();
        Serial.print("RawData : ");
        Serial.println(myRawdata);
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
        if (IrReceiver.decodedIRData.command == 0x10) {
            // do something
        } else if (IrReceiver.decodedIRData.command == 0x11) {
            digitalWrite(LED_PIN, state);
            state = !state;
            delay(3000);
        } else if(IrReceiver.decodedIRData.command == 0x045){
          if (LED_mode != 0){
            LED_mode = 0;
            digitalWrite(LED_PIN, LED_mode);
              
          }
          else{
            LED_mode = 1;
            digitalWrite(LED_PIN, LED_mode);
          }
          delay(2000);
        } 
    }        
}
