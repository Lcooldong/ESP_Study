/*
 * MightyZap.h
 *
 * Created: 2017-02-21 오전 11:26:49
 *  Author: Shim
 */ 

#ifndef IRROBOT_EZController_H_
#define IRROBOT_EZController_H_

#include <MightyZap.h>
#include <Servo.h>
#include "utility/VR.h"
#include "utility/MODE_SW.h"
#include "utility/STEP_VR.h"

#define DATA_ENABLE_PIN 2

#define MODE0_PIN 4
#define MODE1_PIN 12
#define MODE2_PIN 6
#define POSA_PIN 10
#define POSB_PIN 3

class IRROBOT_EZController {
public:
	IRROBOT_EZController();
	IRROBOT_EZController(HardwareSerial  *dev_serial)
	:MightyZap(dev_serial,DATA_ENABLE_PIN),
	VR_1(A1),	VR_2(A4),	VR_3(A5),	VR_4(A0),	VR_5(A2),	VR_6(A3),
	MODE_0(MODE0_PIN),	MODE_1(MODE1_PIN),	MODE_2(MODE2_PIN),	POS_A(POSA_PIN ),	POS_B(POSB_PIN ){};

	IRROBOT_EZController(SoftwareSerial  *dev_serial)
	:MightyZap(dev_serial,DATA_ENABLE_PIN),
	VR_1(A1),	VR_2(A4),	VR_3(A5),	VR_4(A0),	VR_5(A2),	VR_6(A3),
	MODE_0(MODE0_PIN),	MODE_1(MODE1_PIN),	MODE_2(MODE2_PIN),	POS_A(POSA_PIN),	POS_B(POSB_PIN){};

	
	virtual ~IRROBOT_EZController();
	void begin(void);
	void onLED(void);
	void offLED(void);
	void setStep(int step_max, short min, short max);
	int readStep(short val);
	void Mode1(void);
	void Mode2(void);
	void Mode3(void);
	void Mode4(void);
	void Mode5(void);
	void Mode6(void);
	void ModeDefinition(int mode);
	void ModeSelect(int mode1, int mode2, int mode3 , int sw);

	const int userled_Pin =  11;      // the number of the LED pin
	int step_max;
	short step_value_min;
	short step_value_max;

	VR VR_1;
	VR VR_2;
	VR VR_3;
	VR VR_4;
	VR VR_5;
	VR VR_6;

	MODE_SW MODE_0;
	MODE_SW MODE_1;
	MODE_SW MODE_2;
	MODE_SW POS_A;
	MODE_SW POS_B;

	Mightyzap MightyZap;

	Servo servo_CH1;  // create servo object to control a servo channel #1
	Servo servo_CH2;  // create servo object to control a servo channel #2
	Servo servo_CH3;  // create servo object to control a servo channel #3
	Servo servo_CH4;  // create servo object to control a servo channel #4
	Servo servo_CH5;  // create servo object to control a servo channel #5
	Servo servo_CH6;  // create servo object to control a servo channeL #6

private:


};


#endif /* IRROBOT_EZController_H_ */
