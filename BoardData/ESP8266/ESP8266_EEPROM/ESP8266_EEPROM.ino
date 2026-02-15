/*  This code is for the EEPROM in the ESP8266;
     the EEPROM definition in ESP32, Arduino Uno, Mega differs from that of the ESP8266.
    https://pijaeducation.com/eeprom-in-arduino-and-esp/
*/

// include EEPROM Library
#include<EEPROM.h>

/* Defining data storage addresses
   As we know size of int is 4 Bytes,
    If we store int at index 0 then the next EEPROM address can be anything (4,5..10 etc.) after 4 byte address i.e. (0-3)
 You can use this statement to find size of int type data Serial.println(sizeof(int))
*/
int eepromAddr1 = 0, eepromAddr2 = 256;

// We store int and String data in the ESP8266's EEPROM.
int intData = 1044;
String stringData = "My String";

// put is used to write to EEPROM, while read is used to read from it.
void setup() {
  Serial.begin(115200);
  /* Begin with EEPROM by deciding how much EEPROM memory you want to use.
     The ESP8266's maximum EEPROM size is 4096 bytes (4 KB), but we're just using 512 bytes here.
  */
  EEPROM.begin(512);
  delay(500);

  // User's message
  Serial.println("To put int Send → 'int' ");
  Serial.println("To put String Send → 'string' ");
  Serial.println("To get data Send → 'read' ");
  Serial.println("To clear EEPROM Send → 'clear' ");
}

void loop() {
  if (Serial.available()) {
    String x = Serial.readString();
    delay(10);
    Serial.println("");
    Serial.print("Input → ");
    Serial.println(x);

    /* If the user enters 'read' as input,
        we will read from the EEPROM using the 'EEPROM get' function.
    */
    if (x == "read") {
      /* We build a function eeprom_read_string(int),
          where we simply transfer an address to read from and it returns a string
      */
      String si = eeprom_read_string(eepromAddr1);
      /* After reading from EEPROM, convert String to int,
          - 4 is only to see whether it converted or not
      */
      int data = si.toInt();
      Serial.print("Readed Integer: ");
      Serial.println(data - 4);

      String ss = eeprom_read_string(eepromAddr2);
      Serial.print("Readed String: ");
      Serial.println(ss);

      delay(50);
    }
    /* if user send 'string' as input
        then we will write to EEPROM using 'EEPROM put' function
    */
    else if (x == "string") {
      EEPROM.put(eepromAddr2, stringData);
      eeprom_commit();
    }
    /* If the user enters 'int' as input, 
        we convert int to string since we created a function to read string from EEPROM 
        and then use the 'EEPROM put' function to write to EEPROM.
    */
    else if (x == "int") {
      String s = String(intData);
      EEPROM.put(eepromAddr1, s);
      // It's important to use EEPROM.commit() while writing to EEPROM.
      eeprom_commit();
    }
    /* if user send 'clear' as input
        then we will clear data from EEPROM
    */
    else if (x == "clear") {
      for (int i = 0 ; i < 512 ; i++) {
        EEPROM.write(i, 0);
      }
      // Don't forget EEPROM.end();
      EEPROM.end();
      Serial.println("EEPROM Clear Done!");
    }
    /* if user send any other string as input
        then display a message to user
    */
    else {
      Serial.println("Wrong Input");
    }
  }
}

/* Create a type String function that reads from EEPROM byte by byte,
   stores it in a char array, converts it to a string, and returns a String.
*/
String eeprom_read_string(int address) {
  char x[255], c;
  int i = 0;
  while (c != NULL) {
    x[i] = EEPROM.read(address);
    c = x[i];
    address++;
    i++;
  }

  String sr = String(x);
  return sr;
}

// Check whether write to EEPROM was successful or not with the EEPROM.commit() function.
void eeprom_commit() {
  if (EEPROM.commit()) {
    Serial.println("EEPROM successfully committed!");
  } else {
    Serial.println("ERROR! EEPROM commit failed!");
  }
}
