#include <SimpleFOC.h>

BLDCMotor motor = BLDCMotor(11);                               //According to the selected motor, modify the number of pole pairs here, the value in BLDCMotor()
BLDCDriver3PWM driver = BLDCDriver3PWM(32,33,25,22);
  
/// BLDC motor & driver instance
BLDCMotor motor1 = BLDCMotor(11);                              //Also modify the value in BLDCMotor() here
BLDCDriver3PWM driver1  = BLDCDriver3PWM(26,27,14,12);

/// Target Variable
float target_velocity = 5;

/// Serial Command Setting
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&target_velocity, cmd); }

void setup() {
  

  driver.voltage_power_supply = 12;                   //According to the supply voltage, modify the value of voltage_power_supply here
  driver.init();
  motor.linkDriver(&driver);
  motor.voltage_limit = 3;   // [V]                   //According to the supply voltage, modify the value of voltage_limit here
  motor.velocity_limit = 40; // [rad/s]
  
  driver1.voltage_power_supply = 12;                  //Also modify the value of voltage_power_supply here
  driver1.init();
  motor1.linkDriver(&driver1);
  motor1.voltage_limit = 3;   // [V]                  //Also modify the value of voltage_limit here
  motor1.velocity_limit = 40; // [rad/s]

 
  // Open Loop Control Mode Setting
  motor.controller = MotionControlType::velocity_openloop;
  motor1.controller = MotionControlType::velocity_openloop;

  // Initialize the Hardware
  motor.init();
  motor1.init();


  // Add T Command
  // Enter "T+number" in the serial port to set the speed of the two motors.For example, to set the motor to rotate at a speed of 10rad/s, input "T10".
  command.add('T', doTarget, "target velocity");

  Serial.begin(115200);
  Serial.println("Motor ready!");
  Serial.println("Set target velocity [rad/s]");
  _delay(1000);
}

void loop() {
  motor.move(target_velocity);                    //When the motor is powered on, it will rotate at 5rad/s by default
  // motor1.move(target_velocity);

  //User Newsletter
  command.run();
}