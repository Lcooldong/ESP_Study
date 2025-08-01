#include <Arduino.h>
#include <SimpleFOC.h>
// #include <SimpleFOCDrivers.h>
#include <AS5048A.h>

// #define AS5048A_TEST

#define UPDATE_INTERVAL 100
#define DRIVER_VOLTAGE  12
#define LED_BUILTIN      2


#ifdef AS5048A_TEST
const int AS5048A_CS_PIN = 15;
#endif 

uint32_t lastUpdateMillis = 0;
uint32_t count = 0;

float target_angle = 1;
// long timestamp_us = _micros();

MagneticSensorSPIConfig_s AS5048_SPI_CONFIG = {
  .spi_mode = SPI_MODE1,
  .clock_speed = 1000000,
  .bit_resolution = 14,
  .angle_register = 0x3FFF,
  .data_start_bit = 13,
  .command_rw_bit = 14,  // not required
  .command_parity_bit = 15 // parity not implemented
};

MagneticSensorSPI sensor1 = MagneticSensorSPI(15, 14, 0x3FFF); // CS pin 15, 14-bit resolution, angle register 0x3FFF
// MagneticSensorSPI sensor2 = MagneticSensorSPI(AS5048_SPI_CONFIG, 15);

LowsideCurrentSense currentSense1 = LowsideCurrentSense(34, 35, 36, 37); // INA pin 34, INB pin 35, INC pin 36, IND pin 37
BLDCMotor motor1 = BLDCMotor(11);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(4, 16, 17, 5);


#ifdef AS5048A_TEST
AS5048A angleSensor(AS5048A_CS_PIN);
uint16_t zero_position;
uint16_t zero_position_map;
#endif

float inline read2angle(uint16_t angle);
float normalize(float angle);




void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  // SPI.begin(18, 19, 23, 15);
  sensor1.init();
  // sensor2.init();
  motor1.linkSensor(&sensor1);
  
  //▬▬ 

  driver1.init();
  driver1.voltage_power_supply = DRIVER_VOLTAGE;
  motor1.linkDriver(&driver1);

  motor1.controller = MotionControlType::angle;
  motor1.PID_velocity.P = 0.5;
  motor1.PID_velocity.I = 1;
  motor1.PID_velocity.D = 0.005;

  motor1.PID_velocity.output_ramp = 1000;
  motor1.LPF_velocity.Tf = 0.01;
  motor1.P_angle.P = 20;
  motor1.PID_velocity.limit = 10;
  motor1.velocity_limit = 4;
  motor1.voltage_limit = DRIVER_VOLTAGE;

  motor1.init();
  motor1.initFOC();
  
  // motor1.useMonitoring(Serial);
  delay(1000);
#ifdef AS5048A_TEST
  // Serial.printf("MOSI %d  MISO %d SCK  %d ", MOSI, MISO, SCK);
  // SPI.begin();
  angleSensor.begin();


  // Initalize zero position
	zero_position = angleSensor.getRawRotation();
	zero_position_map = read2angle(zero_position);

  Serial.printf("> Zero:  %d\t[%d] \r\n",  zero_position, zero_position_map);
#endif

}

void loop() {
  
  // if(_micros() - timestamp_us > 1e6) {
  //   timestamp_us = _micros();
  //   // inverse angle
  //   target_angle = -target_angle;   
  // }

  motor1.loopFOC();
  // motor1.move(target_angle);
  // sensor1.update();
  // // sensor2.update();
  // Serial.printf("[%5d]:%0.2f [%0.2f] | %0.2f [%02d] - %5.2f\r\n", count, sensor1.getMechanicalAngle()*(180/PI) , sensor1.getSensorAngle(),sensor1.getAngle(),sensor1.getFullRotations(), sensor1.getVelocity());

#ifdef AS5048A_TEST
  uint16_t current_angle = angleSensor.getRawRotation();
	float current_angle_map = read2angle(current_angle);
  float angle = current_angle_map - zero_position_map;
	angle = normalize(angle);
#endif

  uint32_t currentMillis = millis();
  if(currentMillis - lastUpdateMillis >= UPDATE_INTERVAL)
  {
    lastUpdateMillis = currentMillis;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    count++;

    // Serial.printf("[%d] : %d\r\n", count, sensor2.getAngle());


#ifdef AS5048A_TEST
    Serial.printf("[%05d] %5d | %f => %.2f   \r\n", count ,current_angle, current_angle_map, angle);
#endif
  }


}



float inline read2angle(uint16_t angle) {
	/*
	 * 14 bits = 2^(14) - 1 = 16.383
	 *
	 * https://www.arduino.cc/en/Reference/Map
	 *
	 */
	return angle * ((float)360 / 16383);
};

float normalize(float angle) 
{
	// http://stackoverflow.com/a/11498248/3167294
#ifdef ANGLE_MODE_1
	angle += 180;
#endif
	angle = fmod(angle, 360);
	if (angle < 0) {
		angle += 360;
	}
#ifdef ANGLE_MODE_1
	angle -= 180;
#endif
	return angle;
}