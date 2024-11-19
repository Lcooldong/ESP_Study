#include <Arduino.h>
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include <AS5048A.h>

#define AS5048A_TEST

#define UPDATE_INTERVAL 50
#define DRIVER_VOLTAGE 12

const int encoderPWM_0_PIN = GPIO_NUM_14;
const int encoderPWM_1_PIN = 15;
const int pwmMin = 4;
const int pwmMax = 904;

#ifdef AS5048A_TEST
const int AS5048A_CS_PIN = 15;
#endif 

uint32_t lastUpdateMillis = 0;
uint32_t count = 0;

MagneticSensorPWM sensor1 = MagneticSensorPWM(encoderPWM_0_PIN, pwmMin, pwmMax);
void doPWM1(){sensor1.handlePWM();}

#ifdef AS5048A_TEST
AS5048A angleSensor(AS5048A_CS_PIN);
uint16_t zero_position;
uint16_t zero_position_map;
#endif

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

void setup() {
  Serial.begin(115200);
  pinMode(encoderPWM_0_PIN, INPUT);
  sensor1.init();
  sensor1.enableInterrupt(doPWM1);

#ifdef AS5048A_TEST
  Serial.printf("MOSI %d  MISO %d SCK  %d ", MOSI, MISO, SCK);
  // SPI.begin();
  angleSensor.begin();

	zero_position = angleSensor.getRawRotation();
	zero_position_map = read2angle(zero_position);

  Serial.printf("> Zero:  %d\t[%d] \r\n",  zero_position, zero_position_map);
#endif

}

void loop() {
  // int pwmValue =  analogRead(encoderPWM_0_PIN);
  // sensor1.update();
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
    
    count++;

    // Serial.printf("[%d] : %d\r\n", count, sensor1.getAngle());

    // Serial.printf("ERROR : %s\r\n", angleSensor.getErrors());
#ifdef AS5048A_TEST
    Serial.printf("[%05d] %5d | %f => %.2f   \r\n", count ,current_angle, current_angle_map, angle);
#endif
  }


}

