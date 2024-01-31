#include <Arduino.h>
#include <ESP32Servo.h>
#include <DFRobot_TCS3430.h>
#include <Wire.h>
#include "neopixel.h"
#include <WiFi.h>

#define M5STAMP_C3
//#define ATOM_LITE

#define INTERVAL 50
#define COLOR_SENSOR_INTERVAL 50

// PINOUT | 6, 10 NOT ADC
#ifdef M5STAMP_C3
#define EN_PIN            0 // Enable
#define DIR_PIN          18  // Direction
#define STEP_PIN         19  // Step

#define SERVO_PIN         1
#define SERVO_PIN2        18
#define HALL_SENSOR_PIN   4

#define TOUCH_PIN         5

#define OUT_RGB_PIN       6
#define EXTRA_LED_PIN     0

#define COLOR_SDL_PIN     7
#define COLOR_SDA_PIN     8
#define COLOR_LED_PIN     10

#endif

#ifdef ATOM_LITE
#define SERVO_PIN 19
#define HALL_SENSOR_PIN 33
#endif


#ifdef ATOM_LITE
#define HALL_MID_VALUE 2400
#define HALL_TARGET_VALUE 960
#define SERVO_INITIAL_POS 0
#define SERVO_TARGET_POS 150
#endif 

#ifdef M5STAMP_C3
#define COLOR_LED_CHANNEL 0
#define COLOR_Y_MIN_VALUE 300
#define COLOR_Y_MAX_VALUE 700

#define SERVO_INITIAL_POS 0
#define SERVO_TARGET_POS 70
#define SERVO2_INITIAL_POS 10
#define SERVO2_TARGET_POS 40
#define HALL_MID_VALUE 2900
#define HALL_TARGET_VALUE 1310
#define STEPS 1600        // Full Range 10000
#define STEP_DELAY 100
#define INIT_STEP_TIMEOUT 3000
#endif

#define HALL_FAR      0x00
#define HALL_NEARBY   0x04
#define HALL_ARRIVED  0x05
#define SERVO_CLOSED  0x06
#define SERVO_OPENED  0x00
#define COLOR_ON      0x07
#define COLOR_OFF     0x00
#define SERVO_RELEASE 0x00
#define SERVO_PUSH    0x08

enum StepperDirection
{
  FORWARD = 0,
  BACKWARD = 1
};

//StepperDirection stepDir;

typedef struct __attribute__((packed)) packet
{
  uint8_t stx;
  uint8_t servoState;
  uint8_t hallState;
  uint8_t colorState;
  uint8_t buttonState;
  uint8_t etx;
}PACKET;

PACKET dataToSend = {0,};
PACKET buf;

long count = 0;
int hallCount = 0;
int gripperPos = 0;
int buttonPos = 0;
uint64_t lastTime = 0;
uint64_t colorSensorLastTime = 0;
bool colorSensorFlash = false;
uint16_t hallValue;
uint8_t brightnessTestValue = 0;
bool touchToggleFlag = false;
bool servoToggleFlag = false;
bool pressingtouchButton = false;
bool lastTouchValue = true;

Adafruit_NeoPixel* outStrip = new Adafruit_NeoPixel(LED_COUNT, OUT_RGB_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel* extraLED = new Adafruit_NeoPixel(5, EXTRA_LED_PIN, NEO_GRB + NEO_KHZ800);
MyNeopixel* myNeopixel = new MyNeopixel();
Servo gripperServo;
Servo buttonServo;
DFRobot_TCS3430 tcs3430;

void initPacket(PACKET* _packet);
bool sendPacket(uint8_t* _data, size_t len);
void getStatus(int interval);
void initServo();
void rotateServo(Servo *_servo, int targetPos, uint32_t millisecond);
void initStepperMotor();
void moveStepperMotor(int step, bool dir, int stepDelay);
void openServo(bool hallSensor);
void closeServo(bool hallSensor);
void upButtonServo();
void downButtonServo();
void SetOutStripColor(Adafruit_NeoPixel* targetStrip ,uint8_t ledNum, uint32_t color, uint8_t brightness, int wait);
void initTSC3430();
void showColorData();