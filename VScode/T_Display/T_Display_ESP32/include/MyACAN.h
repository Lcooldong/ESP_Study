#ifndef __MY_ACAN_H__
#define __MY_ACAN_H__

#include <Arduino.h>
#include <ACAN_ESP32.h>

const uint32_t DESIRED_BIT_RATE = 500UL * 1000UL ; // 500 kb/s

static ACAN_ESP32 & my_can = ACAN_ESP32::can; // Create a reference to the CAN object

static int my_rx_pin;
static int my_tx_pin;

bool can_init(int rx_pin, int tx_pin);
bool can_send(int id, uint8_t *data, size_t len);
CANMessage can_receive();
bool can_connect();

#endif