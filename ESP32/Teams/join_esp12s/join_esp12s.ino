#include <SoftwareSerial.h>
#define RX_S 7
#define TX_S 6
SoftwareSerial Esp12s =  SoftwareSerial(RX_S, TX_S);

uint8_t PRINTPORT = 0; FILE fOUT = {0,};
#define printr(x, y, args...) PRINTPORT=x,fprintf(&fOUT, y, ##args)
#define printg(x, y, args...) PRINTPORT=x,fprintf(&fOUT, y"\r\n", ##args)
#define AtCIPSENDEX(x) printg(3, "AT+CIPSENDEX=0,%d", x)
static int putchr(char c, FILE* stream)
{
	!PRINTPORT ? Serial.write(c):Esp12s.write(c);
	return(0);
}

int testLED;//LED_BUILTIN(13)
void setup()
{
	fdev_setup_stream(&fOUT, putchr, NULL, _FDEV_SETUP_WRITE);

	Serial.begin(115200);
	//Esp12s.begin(9600);
	//printg(3, "AT+UART_DEF=115200,8,1,0,0");delay(5);
	Esp12s.begin(115200);

	printg(3, "ATE0");delay(5);
	printg(3, "AT+CWMODE=3");delay(5);
	//printg(3, "AT+CWSAP=\"poly\",\"1234\",1,2"); delay(5);
	printg(3, "AT+CIPMUX=1"); delay(5);
	printg(3, "AT+CIPSERVER=1,1009"); delay(5);
	printg(3, "AT+CIPSTO=0"); delay(5);
	printg(3, "AT+CIPMODE=0"); delay(5);
	printg(0, "setup...");

	pinMode(LED_BUILTIN, OUTPUT);
	testLED = HIGH;
	digitalWrite(LED_BUILTIN, testLED);
}

// ******** Test Loop *********
void loop()
{
	int sonarAlt = random(0, 5);

	static uint16_t eTime = 0;
	uint16_t cTime = millis();
	if(cTime - eTime > 500){
		eTime = cTime;

		AtCIPSENDEX(10 + 5 + 2); delay(5);
		printg(3, "sonarAlt: %5d", sonarAlt); delay(10);

		printg(0, "sonarAlt: %5d", sonarAlt);

		//printr(0, ".");
		testLED = !testLED; 
		digitalWrite(LED_BUILTIN, testLED);
	}
	
	while(Esp12s.available() > 0){
		String cmd = Esp12s.readStringUntil(13);
		cmd.trim();//+IPD,1,3:9
#if 1
		printg(0, "recevie: %s", cmd.c_str());
#endif
		if(cmd.length() > 0 && cmd.startsWith("+IPD"))
		{
			int idx = cmd.indexOf(':')+1;
			printg(0, "revCmd: %s", cmd.substring(idx).c_str());
		}
	}
}
