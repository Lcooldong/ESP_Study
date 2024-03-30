/*
 * MightyZap.cpp
 *
 *  Created on: 2016 12. 28.
 *      Author: BG. Shim
 */

#include "Arduino.h"
#include "IRROBOT_EZController.h"

//#include <SoftwareSerial.h>

#define SERVO_CH1_PIN 5
#define DIGITAL_CH1_PIN 7
#define DIGITAL_CH2_PIN 11
#define DIGITAL_CH3_PIN 13
#define ANALOG_CH1_PIN A0 
#define ANALOG_CH2_PIN A2
#define ANALOG_CH3_PIN A3

volatile bool tg = 0;

/*
MightyZap::MightyZap(SoftwareSerial  *dev_serial, int DirectionPin) {
//SoftwareSerial mySerial(receivePin, transmitPin); // RX, TX
}
*/
IRROBOT_EZController::~IRROBOT_EZController() {
	// TODO Auto-generated destructor stub
}

void IRROBOT_EZController::begin(void){


   // initialize the LED pin as an output:
  //pinMode(userled_Pin, INPUT);
 
   MODE_0.begin();
   MODE_1.begin();
   MODE_2.begin();

 servo_CH1.attach(SERVO_CH1_PIN);  // attaches the servo on any pin  to the servo object
 //servo_CH1.attach(DIGITAL_CH1_PIN);

}
void IRROBOT_EZController::onLED(void){
	//digitalWrite(userled_Pin, LOW);
 }

 void IRROBOT_EZController::offLED(void){
	// digitalWrite(userled_Pin, HIGH);
 }

 int IRROBOT_EZController::readStep(short val)
 {
 int step;
 step= (unsigned int)((unsigned long)val*this->step_max/ (this->step_value_max-this->step_value_min));
 return step;
 }

 void IRROBOT_EZController::setStep(int step_max, short min, short max)
 {
	 this->step_max	=step_max;
	 this->step_value_min	=	min;
	 this->step_value_max	=	max;
	 if( this->step_value_min	== this->step_value_max)
	 {
	  this->step_value_min	=	0;
	  this->step_value_max	=	1023;
	 }
 }

void IRROBOT_EZController::Mode1(void)
{
	short position_val;
	position_val = map(VR_1.read(),0,1023,0,4096);
	MightyZap.GoalPosition(0,position_val);
}

void IRROBOT_EZController::Mode2(void)
{

	short position_val;
	short a_pos,b_pos,temp;

	a_pos = map(VR_2.read(),0,1023,0,4096);
	b_pos = map(VR_3.read(),0,1023,0,4096);
	if(a_pos>b_pos)
	{
	  temp = a_pos;
	  a_pos = b_pos;
	  b_pos = temp;
	}
	if(POS_A.isOFF()) position_val = a_pos;
	else if(POS_B.isOFF()) position_val = b_pos;
	MightyZap.GoalPosition(0,position_val);

}

void IRROBOT_EZController::Mode3(void)
{

	short position_val;
	short a_pos,b_pos,temp;

	a_pos = map(VR_2.read(),0,1023,0,4096);
	b_pos = map(VR_3.read(),0,1023,0,4096);
	if(a_pos>b_pos)
	{
	  temp = a_pos;
	  a_pos = b_pos;
	  b_pos = temp;
	}
	if(POS_A.isOFF() || POS_B.isOFF()) tg^=1;
	if(tg) position_val = a_pos;
	else position_val = b_pos;
	MightyZap.GoalPosition(0,position_val);

}

void IRROBOT_EZController::Mode4(void)
{
	short position_val;
	short a_pos,b_pos;
	a_pos = map(VR_2.read(),0,1023,0,4096);
	b_pos = map(VR_3.read(),0,1023,0,4096);
	if(digitalRead(7) == HIGH) position_val = a_pos;
  	else if(digitalRead(11) == HIGH) position_val = b_pos;
  	else position_val = position_val;
	MightyZap.GoalPosition(0,position_val);
}

void IRROBOT_EZController::Mode5(void)
{
	short position_val;
	position_val = map(VR_4.read(),0,1023,0,4095);
	MightyZap.GoalPosition(0,position_val);
}

void IRROBOT_EZController::Mode6(void)
{
	short position_val;
	short a_pos,b_pos;
	char data;
	a_pos = map(VR_2.read(),0,1023,0,4096);
	b_pos = map(VR_3.read(),0,1023,0,4096);

	SoftwareSerial userSerial(8,9);
	Serial.begin(9600);
	userSerial.begin(9600);
	
	if(POS_A.isOFF()) userSerial.write('A');
	else if(POS_B.isOFF()) userSerial.write('B');

	if(userSerial.available()>0) data = userSerial.read();

	if(data=='A')
	{
		position_val = a_pos;
    	Serial.println("'A' Recieved");
	}
	else if(data=='B')
	{
	    position_val = b_pos;
	    Serial.println("'B' Recieved");
	}
	MightyZap.GoalPosition(0,position_val);
}

void IRROBOT_EZController::ModeDefinition(int mode)
{
	switch(mode)
	{
		case 1 : Mode1(); break;
		case 2 : Mode2(); break;
		case 3 : Mode3(); break;
		case 4 : Mode4(); break;
		case 5 : Mode5(); break;
		case 6 : Mode6(); break;
		default : break;
	}
}

 void IRROBOT_EZController::ModeSelect(int mode1, int mode2, int mode3 , int sw)
 {
	switch(sw)
	{
		case 1 : ModeDefinition(mode1); break;
		case 2 : ModeDefinition(mode2); break;
		case 3 : ModeDefinition(mode3); break;
		default : break;
	}
 }
 