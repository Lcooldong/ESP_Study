/*
 * VR.cpp
 *
 * Created: 2017-02-21 오전 11:24:50
 *  Author: Shim
 */ 
 #include "Arduino.h"
 #include "VR.h"
 VR::VR(int pin)
 {
   this->pin=pin;
 }
  VR::~VR()
  {
	 // TODO Auto-generated destructor stub
  }
 short VR::read(void)
 {
   return analogRead(this->pin);
 }
 /*
 short VR::read(void)
 {
	 this->val = analogRead(this->pin);
 }
 */