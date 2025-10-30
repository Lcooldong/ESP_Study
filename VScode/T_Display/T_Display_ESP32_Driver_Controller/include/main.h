#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>

#include <HardwareSerial.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <core_version.h> // For ARDUINO_ESP32_RELEASE
#include <RotaryEncoder.h> // Include the RotaryEncoder library
#include <AS5600.h>
// #include <NimBLEServer.h>
// #include <NimBLEDevice.h>


#include "MyLittleFS.h" // #include <FS.h>
#include "MyACAN.h"
#include "MyTFT.h"
#include "MyButton.h"
#include "MyBLECallback.h"

#define BUTTON_PIN     GPIO_NUM_38 // GPIO pin for the button

#define ROTARY_CLK_PIN GPIO_NUM_32
#define ROTARY_DT_PIN  GPIO_NUM_33
#define ROTARY_SW_PIN  GPIO_NUM_37 // 34~39 pull-up impossible (use external pullup)

#define CAN_RX_PIN     GPIO_NUM_25 // CAN RX pin
#define CAN_TX_PIN     GPIO_NUM_26 // CAN TX pin

#define LED_PIN        GPIO_NUM_17 // GPIO pin for the LED (not used in this example)
#define LED_CHANNEL    0 // LED channel for PWM (not used in this example)
#define LED_FREQUENCY  5000 // Frequency for PWM (not used in this example)
#define LED_RESOLUTION 8 // Resolution for PWM (not used in this example)

#define AS5600_SDA_PIN GPIO_NUM_21
#define AS5600_SCL_PIN GPIO_NUM_22
#define AS5600_DIR_PIN GPIO_NUM_27

#define UART1_RX_PIN  GPIO_NUM_13
#define UART1_TX_PIN  GPIO_NUM_15

void encoder_update();
void indicate_led();

#endif