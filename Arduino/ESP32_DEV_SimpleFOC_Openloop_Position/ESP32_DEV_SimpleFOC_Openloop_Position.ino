#include <SimpleFOC.h>
#define RESISTANCE 5.6

BLDCMotor motor = BLDCMotor(11);         //According to the selected motor, modify the value in BLDCMotor()
BLDCDriver3PWM driver = BLDCDriver3PWM(32, 33, 25, 22);

BLDCMotor motor1 = BLDCMotor(11);        //Also modify the value of pole logarithm here
BLDCDriver3PWM driver1 = BLDCDriver3PWM(26, 27, 14, 12);

//Target Variable
float target_velocity = 0;

//Serial Command Setting
Commander command = Commander(Serial);E
void doTarget(char* cmd) { command.scalar(&target_velocity, cmd); }

void setup() {

  driver.voltage_power_supply = 12;       //According to the supply voltage of the motor, modify the value of the voltage_power_supply variable
  driver.init();
  motor.linkDriver(&driver);
  motor.voltage_limit = 12;   // [V]      //According to the supply voltage of the motor, modify the value of the voltage_limit variable
  motor.velocity_limit = 15; // [rad/s]
  
  // driver1.voltage_power_supply = 12;
  // driver1.init();
  // motor1.linkDriver(&driver1);
  // motor1.voltage_limit = 12;   // [V]
  // motor1.velocity_limit = 15; // [rad/s]

 
  //Open Loop Control Mode Setting
  motor.controller = MotionControlType::angle_openloop;
  // motor1.controller = MotionControlType::angle_openloop;

  //Initialize the Hardware
  motor.init();
  motor.useMonitoring(Serial);

  // motor1.init();
  // motor1.useMonitoring(Serial);

  //Add T Command (Set New Line)
  //Enter the "T+number" command in the serial monitor and click send
  //to control the two motors to rotate to the specified position
  //For example, T3.14 is to rotate to the position of 180° in the positive direction
  command.add('T', doTarget, "target velocity");

  Serial.begin(115200);
  Serial.println("Motor ready!");
  Serial.println("Set target velocity [rad/s]");
  _delay(1000);
}

void loop() {
  motor.move(target_velocity);
  // motor1.move(target_velocity);

  //User Newsletter
  command.run();
}