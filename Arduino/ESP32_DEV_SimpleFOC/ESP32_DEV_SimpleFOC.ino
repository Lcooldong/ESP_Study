#include <SimpleFOC.h>

// GM3506
#define POLE_PAIR    11  // 22/2 = 11
#define LOAD_CURRENT 1   // 1A
#define MAX_VOLTAGE  16
#define LOAD_VOLTAGE 12
#define RESISTANCE   5.6 // 5.6 Ω

uint32_t currentMillis = 0;
uint32_t previousMillis[4] = {0,};
int counter = 0;

// BLDC motor & driver instance
// BLDCMotor motor = BLDCMotor(pole pair number);
BLDCMotor motor = BLDCMotor(11);
// BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
BLDCDriver3PWM driver = BLDCDriver3PWM(4, 16, 17, 5);

// Stepper motor & driver instance
//StepperMotor motor = StepperMotor(50);
//StepperDriver4PWM driver = StepperDriver4PWM(9, 5, 10, 6,  8);

//target variable
float target_position = 0;

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&target_position, cmd); }
void doLimit(char* cmd) { command.scalar(&motor.voltage_limit, cmd); }
void doVelocity(char* cmd) { command.scalar(&motor.velocity_limit, cmd); }

void setup() {

  // use monitoring with serial 
  Serial.begin(115200);
  // enable more verbose output for debugging
  // comment out if not needed
  SimpleFOCDebug::enable(&Serial);

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  // limit the maximal dc voltage the driver can set
  // as a protection measure for the low-resistance motors
  // this value is fixed on startup
  driver.voltage_limit = 6;
  if(!driver.init()){
    Serial.println("Driver init failed!");
    return;
  }
  // link the motor and the driver
  motor.linkDriver(&driver);

  // limiting motor movements
  // limit the voltage to be set to the motor
  // start very low for high resistance motors
  // currnet = voltage/resistance, so try to be well under 1Amp
  motor.voltage_limit = 3;   // [V]
  // limit/set the velocity of the transition in between 
  // target angles
  motor.velocity_limit = 5; // [rad/s] cca 50rpm
  
  // open loop control config
  motor.controller = MotionControlType::angle_openloop;

  // init motor hardware
  if(!motor.init()){
    Serial.println("Motor init failed!");
    return;
  }

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
  // // open  loop angle movements
  // // using motor.voltage_limit and motor.velocity_limit
  // // angles can be positive or negative, negative angles correspond to opposite motor direction
  motor.move(target_position);
  
  // user communication
  command.run();
}