// https://github.com/Chris--A/Keypad
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns

// https://lastminuteengineers.com/arduino-keypad-tutorial/
// PIN 4x3 Membrane Keypad
// R1, R2, R3, R4, C1, C2, C3 


char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
//byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {5, 4, 3}; //connect to the column pinouts of the keypad


////Wemos D1 mini
//byte rowPins[ROWS] = {23, 19, 18, 26}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {32, 25, 27}; //connect to the column pinouts of the keypad


//ESP32 Dev module 38Pin
byte rowPins[ROWS] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 33, 32}; //connect to the column pinouts of the keypad


//Create an object of keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(115200);
}
  
void loop(){
  char key = keypad.getKey();// Read the key
  
  // Print if key pressed
  if (key){
    Serial.print("Key Pressed : ");
    Serial.println(key);
  }
}
