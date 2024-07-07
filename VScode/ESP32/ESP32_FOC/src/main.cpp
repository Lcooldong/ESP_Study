#include <Arduino.h>

/********************************************************************************************************
 *  Title: ELECTRONOOBS open source FOC controller. 
 * Date: 25/05/2024
 * Version: 2.5
 * Author: http://electronoobs.com
 * Based on tutorial: https://www.electronoobs.com/eng_arduino_tut198.php

 * This is a sensored ESC with FOC implemented based on Arduino with the ATmega328 chip. 
 * It uses SPWM signals and PID to control the position of brushless motors.
 * Subscribe: https://youtu.be/pzGRKyHlXsM
********************************************************************************************************/

#include <Wire.h>
#include <AS5600.h>
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
  #define SYS_VOL   3.3
#else
  #define SERIAL Serial
  #define SYS_VOL   5
#endif

//Variables for the hall effect angle sensor
AMS_5600 ams5600;
int ang, lang = 0;


//Inputs or outputs
const int ledChannel0 = 0;
const int ledChannel1 = 1;
const int freq = 31250;
const int resolution = 8;

const int potentiometer = A3;     // pot controls the RPM speed
const int MotorPinPhase_A =9;     //Driver's pin for coil A
const int MotorPinPhase_B =10;    //Driver's pin for coil B
const int MotorPinPhase_C =11;    //Driver's pin for coil C
const int buzzer = GPIO_NUM_14;            //Pin for the on board buzzer
const int fanPin = 5;             //Pin for fan MOSFET
const int supplyVoltagePin = GPIO_NUM_15;  //Pin for reading the supply voltage
const int temperaturePin = A0;    //Pin for reading the temperature with the NTC
const int PWMSensor = 2;          //PWM output from the magnetic encoder sensor (not used in this code...)

//The coils have a phase difference of 120 degrees so between each coil we have 120
//Here is where we will store the SPWM value for each coil
int SPWMValueA=0;  
int SPWMValueB=120; 
int SPWMValueC=240; 


//Variables the user could change depending on teh used motor ...
const int motorPoles = 11;      //PP value is pole pair (S and N) of the motor. (22 magnets = 11 pp) 
int TorqueValue=100;            //Set torque value from 0 to 100%;


/*The motor has ¡the real position in degrees from 0 to 360 but also the electrical position
 * The electrical position is affected by the amount of "pp" or pole pairs. In this case my motor
 * has 11 pairs, so for 360 degrees of real shaft rotation, we need 11 electrical rotations */
int RealShaftPosition=0;        //Here we store the read position of shaft from the sensor
int ElectricalPosition=0;       //Here we store the electrical position of shaft
int RotationDirection=0;        //Magnetic field orientation vector so we know if we rotate CW or CCW

//PID Variables
float PID_P = 0;                //Here we stopre the P gain
float PID_D = 0;                //Here we stopre the D gain
float Kp = 25;                  //P is the proportional gain for the PID control (no I yet...)
float Kd = 32;                  //D is the derivative gain for the PID control (no I yet...)
float PID_setpoint=0;           //Here we store the setpoint for the PID calculations
float PID_error=0;              //Here we store the error for the PID calculations (error = real_value - setpoint)
float previousPID_error=0;      //This is used for the D part of the PID calculations           
float elapsedTime, currentTime, previousTime;        //Variables for time control
int PID_Torque=0;               //Here we will store the tortal PID torque affected by the PID code

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  ledcSetup(ledChannel0, freq, resolution);
  // ledcSetup(ledChannel1, freq, resolution);

  ledcAttachPin( MotorPinPhase_A, ledChannel0);
  ledcAttachPin( MotorPinPhase_B, ledChannel0);
  ledcAttachPin( MotorPinPhase_C, ledChannel0);

  // TCNT : Timer/Counter Register
  // OCR : Output Compare Register
  // ICR : Input Capture Register  (for 16bit Timer)
  // TIMSK : Timer/Counter Interrupt Mask Register
  // TIFR : Timer/Counter Interrupt Flag Register
  // atmega168 -> 16MHz, Timer0,2 8bit | Timer1,3 16bit 
  // TCCR1B(Timer1)  TCCR2B(Timer2)
  // 0b11111000은 CS1n 값을 초기화하고 0x01 은 No Prescaling 을 의미 (x1)
  // TCCR1B = TCCR1B & 0b1111,1000 | 0x01;  // fix PWM frequency at 31250 Hz for Pins 9 and 10   
  // TCCR2B = TCCR2B & 0b1111,1000 | 0x01;  // fix PWM frequency at 31250 Hz for Pins 11 and 3
  // ICR1 = 255 ;                          // 8 bit resolution
 
  pinMode(potentiometer, INPUT);        //Set pot pin as input
  pinMode(supplyVoltagePin, INPUT);     //Set voltage in pin as input
  pinMode(temperaturePin, INPUT);       //Set temperature read pin as input
  // pinMode(MotorPinPhase_A, OUTPUT);     //Set driver pins as outputs
  // pinMode(MotorPinPhase_B, OUTPUT);     //Set driver pins as outputs
  // pinMode(MotorPinPhase_C, OUTPUT);     //Set driver pins as outputs
  pinMode(buzzer, OUTPUT);              //Set buzzer as output
  pinMode(fanPin, OUTPUT);              //Set fanPin as output

  digitalWrite(buzzer, LOW);            //Start with the buzzer off
  digitalWrite(fanPin, LOW);            //Start with the fan off (temperature not yet implemented)

  //Start the magnetic sensor and check for magnet...
  if(ams5600.detectMagnet() == 0 ){
    while(1){
      if(ams5600.detectMagnet() == 1 ){
        SERIAL.print("Current Magnitude: ");
        SERIAL.println(ams5600.getMagnitude());
        break;
      }
      else{
        SERIAL.println("Can not detect magnet");
      }
      delay(200);
    }
  }//End of setting the magnet sensor
  currentTime = millis();

  //tone(buzzer, 2000, 1000);
}//End of SETUP LOOP





void loop() {  
  //Read the setpoint from the potentiometer
  PID_setpoint = analogRead(potentiometer);                             //Read the analog value of the potentiometer
  PID_setpoint = map(PID_setpoint,0,1023,0,360);                        //Map that value to a range of 0 to 360º

  //Read the real angle from the sensor
  float rawangle = ams5600.getRawAngle();                               //RAW read from the sensor (not in degrees yet)
  float realAngle = rawangle * 0.087890625;                             //Value from sensor datasheet
  realAngle = constrain(realAngle, 0, 360);                             //Constraint the values to a range of 0 to 360º
  
  ElectricalPosition=map(realAngle,0,360,0,(360*motorPoles));           //We need to scale the electrical position depending on the amount of pole pairs
  ElectricalPosition=constrain(ElectricalPosition, 0, 360*motorPoles);  //Constraint the values once again
  RealShaftPosition=map(ElectricalPosition,0,(360*motorPoles),0,360); 
  
  // We first calculate the PID error between the value we want and the value we have (in degrees)
  PID_error=RealShaftPosition-PID_setpoint;                             //First as always, wec alculate teh PID error

  //Decide if we go CW or CCW
  if (PID_error<0){RotationDirection = 90;}                             //Magnetic field vector is now 90 degrees ahead so CW
  if (PID_error>0){RotationDirection = 270;}                            //Magnetic field vector is now 90 degrees behind so CCW

  
  ////////////PID Calculations///////////////
  //Calculate the P gain
  PID_P = Kp * PID_error;

  //Calculate the D gain
  currentTime = millis();
  elapsedTime = currentTime - previousTime;
  PID_D = Kd*((PID_error - previousPID_error)/elapsedTime);
  previousTime = currentTime;
  previousPID_error = PID_error;
  
  //Sum up P and D gains
  PID_Torque=abs(PID_P + PID_D);                                      //Define the gain of the PID system. 
  PID_Torque=constrain(PID_Torque,0,TorqueValue);                     //Limit the torque 0-100%. The driver might get HOT so lower the value to maybe 50% or so...
  
  rotate();   //Run the function that will apply the SPWM values to the driver        
         
}//End of VOID LOOP



  
void rotate(){
  const int SyncOffset=21;                                              //You might neet do adjust this value (offset between where the real angle starts and the electrical angle starts)
  SPWMValueA = ElectricalPosition + RotationDirection + SyncOffset;     //We add the position and direction so it will rotate CW or CW
  SPWMValueB = SPWMValueA + 120;                                        //120 degrees of phase difference between coils A and B 
  SPWMValueC = SPWMValueB + 120;                                        //120 degrees of phase difference between coils B and C 
 
  SPWMValueA = SPWMValueA%360;                                          //Keep the values between 0 and 360 degrees
  SPWMValueB = SPWMValueB%360;
  SPWMValueC = SPWMValueC%360;

  
  //Calculate the PWM values for creating a sine signal (SPWM)
  int SINE_A_PWM = sin((double)SPWMValueA*PI/360)*127.5+127.5;          //Multiply by PI and divide by 180 in order to pass from degrees to radians
  int SINE_B_PWM = sin((double)SPWMValueB*PI/360)*127.5+127.5;          //Multiply by 127.5 and add 127.5 in order to keep the range between 0-255 (8 bit PWM signal)
  int SINE_C_PWM = sin((double)SPWMValueC*PI/360)*127.5+127.5;          //SINE values between -1 and 1 are placed between 0-255 for PWM. 

   
  //Write SPWM values to each pin of the driver
  // analogWrite(MotorPinPhase_A, SINE_A_PWM*PID_Torque/100);
  // analogWrite(MotorPinPhase_B, SINE_B_PWM*PID_Torque/100);
  // analogWrite(MotorPinPhase_C, SINE_C_PWM*PID_Torque/100);

  ledcWrite(MotorPinPhase_A, SINE_A_PWM*PID_Torque/100);
  ledcWrite(MotorPinPhase_B, SINE_B_PWM*PID_Torque/100);
  ledcWrite(MotorPinPhase_C, SINE_C_PWM*PID_Torque/100);
    
}//End of moving function