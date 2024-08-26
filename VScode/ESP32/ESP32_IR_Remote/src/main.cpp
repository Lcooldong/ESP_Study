#include <Arduino.h>
// #include <IRremote.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

const int RECV_PIN = 25;
const int SEND_PIN = 26;
const int BUTTON_PIN = 27;

IRrecv irrecv(RECV_PIN);
IRsend irsend(SEND_PIN);
decode_results results;

unsigned long countTime = 0;
unsigned long count = 0;

unsigned long irTime = 0;


const uint32_t eighteenDegree = 0x880831C;
const uint32_t thirtyDegree = 0x8808F18;
uint32_t currentDegree = 0x880831C;

uint8_t currentIndex = 0;
uint32_t degrees[13] = {
  0x880831C,
  0x880841D,
  0x880851E,
  0x880861F,
  0x8808710,
  0x8808811,
  0x8808912,
  0x8808A13,
  0x8808B14,
  0x8808C15,
  0x8808D16,
  0x8808E17,
  0x8808F18
};



uint16_t* lastRecv;
uint16_t* lastBuf;
uint16_t lastLen;
uint64_t lastValue;
uint16_t lastBits;

decode_results lastResults;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  irrecv.enableIRIn(); // Start the receiver
  irsend.begin();
  Serial.println("Enabled IRin");
  Serial.printf("Value : %d\r\n", sizeof(degrees)/sizeof(uint32_t));
}

void loop() {

  if(millis() - irTime > 100)
  {
    
    if (irrecv.decode(&results)) {
      // Serial.printf( "Received : 0x%X \r\n",  results.value); // Only One can read
      serialPrintUint64(results.value, HEX);
      Serial.println("");

      lastResults = results;

      Serial.printf("Bits : %d | address : %d  | %X\r\n", results.bits, results.decode_type, results.state);


      switch (lastResults.decode_type)
      {
      case SAMSUNG:
        Serial.println("SAMSUNG");

        for (int i = 0; i < lastResults.rawlen; i++)
        {
          
          Serial.printf("%4d |", lastResults.rawbuf[i]);
        }
        Serial.println("\r\n------------------------------");
        break;
      case LG:
        Serial.println("LG");
        break;
      default:
        break;
      }
   

      irrecv.resume(); // Receive the next value
    } 

    irTime = millis();
  }
  
  if(millis() - countTime > 1000)
  {
    countTime = millis();
    // Serial.printf("CNT : %d\r\n", count++);
  }

  int btnRead = digitalRead(BUTTON_PIN);
  if(!btnRead)
  {
    Serial.println("Button Read");
    Serial.printf("BTN[%d] \r\n", lastResults.rawlen);
    irsend.sendRaw((uint16_t*)lastResults.rawbuf, lastResults.rawlen, 38);
    switch (lastResults.decode_type)
    {
    case SAMSUNG:
      irsend.sendSAMSUNG(lastResults.value, lastResults.bits, 2);
      Serial.println("Send SAMGSUNG");
      break;
    case LG:
      irsend.sendLG(lastResults.value, lastResults.bits, 2);
      Serial.println("Send LG");
      break;  
    default:
      break;
    }
    delay(500);
  }

  if(Serial.available())
  {
    char c = Serial.read();
    switch (c)
    {
    case '1':
    {
      irsend.sendLG2(0x88C0051, 28, 2);
      Serial.printf("Send 1 - Turn Off\r\n");
      break;
    }
    
    case '2':
      irsend.sendLG2(0x880190A, 28, 2);
      Serial.printf("Send 2 - Recent Turn On\r\n");
      break;
    case '3':
      irsend.sendLG2(0x8801C0D, 28, 2);
      Serial.printf("Send 3 - Turn On\r\n");
      break;
    case '4':
      irsend.sendLG2(0x8809902, 28, 2);
      Serial.printf("Send 4 - Remove Humidity\r\n");
      break;
    case '5':
      irsend.sendLG2(0x8808A13, 28, 2);
      Serial.printf("Send 4 - Cooling Default 25'c\r\n");
      break;
    case 'u':
      if( currentDegree <= thirtyDegree)
      {
        if(currentIndex < sizeof(degrees)/sizeof(uint32_t))
        {
          currentIndex++;
        currentDegree = degrees[currentIndex];
        }
      }

      irsend.sendLG2(currentDegree, 28, 2);
      Serial.printf("Send Up - Cooling 0x%X\r\n", currentDegree);
      break;
    case 'd':
      if( currentDegree >= eighteenDegree)
      {
        if(currentIndex >0)
        {
          currentIndex--;
          currentDegree = degrees[currentIndex];
        }      
      }

      irsend.sendLG2(currentDegree, 28, 2);
      Serial.printf("Send Down - Cooling 0x%X\r\n", currentDegree);
      break;
    case 't':
      break;

    default:
      break;
    }
  }
}

