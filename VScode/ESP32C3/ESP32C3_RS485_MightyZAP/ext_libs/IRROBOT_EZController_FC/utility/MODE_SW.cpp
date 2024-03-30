/*
 * MODE_SW.cpp
 *
 * Created: 2017-02-21 오전 11:26:49
 *  Author: Shim
 */ 

 #include "Arduino.h"
 #include "MODE_SW.h"

 MODE_SW::MODE_SW(int pin)
 {
	 this->pin=pin;
 }
  MODE_SW::~MODE_SW()
  {
	  // TODO Auto-generated destructor stub
  }
 void MODE_SW::begin(void)
 {
    // initialize the pushbutton pin as an input:
	pinMode(this->pin, INPUT);
 }
 short MODE_SW::read(void)
 {
	 return digitalRead(this->pin);
 }
  bool MODE_SW::isON(void)
  {
	  if(digitalRead(this->pin)==HIGH) return true;
	 
	 return false;
  }
  bool MODE_SW::isOFF(void)
  {
	  if(digitalRead(this->pin)==LOW) return true;
	 
	 return false;
  }