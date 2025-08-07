#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include "encoders/smoothing/SmoothingSensor.h"


// GM3506
#define POLE_PAIR            11  // 22/2 = 11
#define LOAD_CURRENT         1   // 1A
#define MAX_VOLTAGE          16
#define LOAD_VOLTAGE         12
#define DRIVER_VOLTAGE       12
#define MOTOR_RESISTANCE     5.6 // 5.6 Ω

#define CURRENT_GAIN         50
#define SHUNT_RESISTACE      0.01

uint32_t currentMillis = 0;
uint32_t previousMillis[4] = {0,};
int counter = 0;

// CS pin 15, 14-bit resolution, angle register 0x3FFF
MagneticSensorSPI sensor1 = MagneticSensorSPI(5, 14, 0x3FFF);

// MagneticSensorI2C sensor1 = MagneticSensorI2C(AS5600_I2C);
// MagneticSensorI2C sensor2 = MagneticSensorI2C(AS5600_I2C);
// TwoWire I2Cone = TwoWire(0);  // i2c bus num 0
// TwoWire I2Ctwo = TwoWire(1);  

InlineCurrentSense current_sense1 = InlineCurrentSense(SHUNT_RESISTACE, CURRENT_GAIN, 39, 36);
InlineCurrentSense current_sense2 = InlineCurrentSense(SHUNT_RESISTACE, CURRENT_GAIN, 35, 34);

BLDCMotor motor1 = BLDCMotor(POLE_PAIR);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(32,33,25,22);

BLDCMotor motor2 = BLDCMotor(POLE_PAIR); 
BLDCDriver3PWM driver2  = BLDCDriver3PWM(26,27,14,12);

// SmoothingSensor smooth = SmoothingSensor(sensor1, motor1);

//target variable
float target_position = 0;

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&target_position, cmd); }
void doLimit(char* cmd) { command.scalar(&motor1.voltage_limit, cmd); }
void doVelocity(char* cmd) { command.scalar(&motor1.velocity_limit, cmd); }

void setup() {

  // use monitoring with serial 
  Serial.begin(115200);
  // enable more verbose output for debugging
  // comment out if not needed
  SimpleFOCDebug::enable(&Serial);

  
  sensor1.init();
  motor1.linkSensor(&sensor1);  //Closed-Loop

  // driver config
  // power supply voltage [V]
  driver1.voltage_power_supply = DRIVER_VOLTAGE;
  driver1.voltage_limit = 8;
  if(!driver1.init()){
    Serial.println("Driver init failed!");
    return;
  }
  // link the motor and the driver
  motor1.linkDriver(&driver1);

  // current_sense1.linkDriver(&driver1);
  // if(!current_sense1.init()){
  //   Serial.println("Current sense init failed.");
  //   return;
  // }
  // current_sense1.gain_b *= -1; // B상 이득 부호 반전
  // motor1.linkCurrentSense(&current_sense1);

  // // limiting motor movements
  // // limit the voltage to be set to the motor
  // // start very low for high resistance motors
  // // currnet = voltage/resistance, so try to be well under 1Amp
  // motor1.controller = MotionControlType::velocity;
  motor1.PID_velocity.P = 0.2;     
  motor1.PID_velocity.I = 20;
  motor1.PID_velocity.D = 0.001;
  motor1.PID_velocity.output_ramp = 1000;
  motor1.LPF_velocity.Tf = 0.01;

  motor1.voltage_limit = 8;   // [V]
  motor1.current_limit = 1;
  motor1.velocity_limit = 100; // [rad/s] cca 50rpm


  // motor1.voltage_sensor_align = 1;
  // motor1.phase_resistance = MOTOR_RESISTANCE;
  // motor1.foc_modulation = FOCModulationType::SpaceVectorPWM; // default SinePWM
  // motor1.controller = MotionControlType::torque;
  // motor1.torque_controller = TorqueControlType::foc_current;
  // motor1.controller = MotionControlType::angle;
  motor1.controller = MotionControlType::velocity;
  

  motor1.useMonitoring(Serial);  // motor1.monitor();
  // init motor hardware
  if(!motor1.init()){
    Serial.println("Motor init failed!");
    return;
  }

  // motor1.sensor_direction = Direction::CCW;
  motor1.initFOC();

  // add target command T
  command.add('T', doTarget, "target angle");
  command.add('L', doLimit, "voltage limit");
  command.add('V', doLimit, "movement velocity");

  Serial.println("Motor ready!");
  Serial.println("Set target position [rad]");
  _delay(1000);
}

void loop() {
  currentMillis = millis();
  if(currentMillis - previousMillis[0]>= 1000)
  {
    previousMillis[0] = currentMillis;
    counter++;
    Serial.printf("[%d]\r\n", counter);
  }
  
  motor1.loopFOC();
  motor1.move(target_position);
  // sensor1.update();
  
  // user communication
  command.run();

  motor1.monitor();
}