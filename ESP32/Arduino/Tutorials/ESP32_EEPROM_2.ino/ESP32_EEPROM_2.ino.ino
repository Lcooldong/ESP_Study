/* This program is for EEPROM in ESP32,
   EEPROM concept in ESP8266 or nodeMCU and Arduino is different
   https://pijaeducation.com/eeprom-in-arduino-and-esp/
*/

// include EEPROM Library
#include<EEPROM.h>

/* Defining data storage addresses
   As we know size of int is 4 Bytes,
    If we store int at index 0 then the next EEPROM address can be anything (4,5..10 etc.) after 4 byte address i.e. (0-3)
  You can use this statement to find size of int type data Serial.println(sizeof(int))
*/
int eepromAddr1 = 0, eepromAddr2 = 10;

// int and string data which we store in EEPROM of Arduino Uno or Arduino Nano etc.
int intData = 1024;
String stringData = "My String";

// put means writing to EEPROM and get means reading from EEPROM
void setup() {
  Serial.begin(115200);
  /* Begin with EEPROM by deciding how much EEPROM memory you want to use.
    The ESP32's maximum EEPROM size is 4096 bytes (4 KB), but we're just using 512 bytes here.
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
    String str = Serial.readString();
    delay(10);
    Serial.println("");
    Serial.print("Input → ");
    Serial.println(str);

    /* If the user enters 'read' as input,
        we will read from the EEPROM using 'readInt and readString' function.
    */
    if (str == "read") {
      int rInt = EEPROM.readInt(eepromAddr1);
      String rString = EEPROM.readString(eepromAddr2);
      Serial.println("Read from EEPROM");
      Serial.print("Int:");
      Serial.println(rInt);
      Serial.print("Str:");
      Serial.println(rString);
    }
    /* if user send 'int' as input
        then we will write to EEPROM using 'EEPROM.writeInt' function
    */
    else if (str == "int") {
      EEPROM.writeInt(eepromAddr1, intData);
      EEPROM.commit();
      Serial.println("intData write in EEPROM is Successful");
    }
    /* if user send 'string' as input
        then we will write to EEPROM using 'EEPROM.writeString' function
    */
    else if (str == "string") {
      EEPROM.writeString(eepromAddr2, stringData);
      EEPROM.commit();
      Serial.println("stringData write in EEPROM is Successful");
    }
    /* if user send 'clear' as input
        then we will clear data from EEPROM
    */
    else if (str == "clear") {
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
      // Don't forget EEPROM.end();
      EEPROM.end();
      Serial.println("EEPROM Clear Done!");
    }
    // Provide a reply if the input does not match.
    else {
      Serial.println("Invalid Input");
    }
  }
}
