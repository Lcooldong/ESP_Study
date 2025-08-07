#include <SimpleFOC.h>

uint32_t lastMillis = 0;
uint32_t currentMillis = 0;
// ADC
InlineCurrentSense current_sense0 = InlineCurrentSense(0.01f, 50.0f, 39, 36);  // Shunt resistor, Gain, A,B 측정 ADC 핀 -> I_c = - I_a - I_b
InlineCurrentSense current_sense1 = InlineCurrentSense(0.01f, 50.0f, 35, 34);

BLDCMotor motor = BLDCMotor(11);  //According to the selected motor, modify the number of pole pairs here, the value in BLDCMotor()
BLDCDriver3PWM driver = BLDCDriver3PWM(32, 33, 25, 22);

float target_velocity = 5;

Commander command = Commander(Serial);
void doTarget(char* cmd) {
  command.scalar(&target_velocity, cmd);
}

void setup() {
  Serial.begin(115200);
  SimpleFOCDebug::enable(&Serial);
  analogReadResolution(12);  // ESP32

  driver.voltage_power_supply = 12;  //According to the supply voltage, modify the value of voltage_power_supply here
  driver.init();
  motor.linkDriver(&driver);
  motor.voltage_limit = 3;    // [V]                   //According to the supply voltage, modify the value of voltage_limit here
  motor.velocity_limit = 40;  // [rad/s]

  motor.controller = MotionControlType::velocity_openloop;
  motor.init();

  command.add('T', doTarget, "target velocity");

  if (!current_sense0.init()) {
    Serial.println("Current sense init failed.");


    return;
  }
  current_sense1.init();

  current_sense0.gain_b *= -1;  // B상 이득 부호 반전
  current_sense1.gain_b *= -1;

  Serial.println("Current sense ready.");
}

void loop() {
  motor.move(target_velocity);
  command.run();





  currentMillis = millis();
  if (currentMillis - lastMillis >= 100) {
    lastMillis = currentMillis;
    PhaseCurrent_s currents0 = current_sense0.getPhaseCurrents();
    float current_magnitude0 = current_sense0.getDCCurrent();
    // PhaseCurrent_s currents1 = current_sense1.getPhaseCurrents();
    // float current_magnitude1 = current_sense1.getDCCurrent();
    Serial.printf("A: %5.1f |B:%5.1f |DC:%5.1f\r\n", currents0.a * 1000, currents0.b * 1000, current_magnitude0 * 1000);

    // Serial.print(currents1.a*1000); // milli Amps
    // Serial.print("\t");
    // Serial.print(currents1.b*1000); // milli Amps
    // Serial.print("\t");
    // Serial.print(currents1.c*1000); // milli Amps
    // Serial.print("\t");
    // Serial.println(current_magnitude1*1000); // milli Amps
    // Serial.println();
  }
}