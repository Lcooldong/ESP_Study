#ifndef __ESP_CAN_H__
#define __ESP_CAN_H__

#include <Arduino.h>
#include <ACAN_ESP32.h>

// Version Flash
#include <core_version.h>
#include <esp_flash.h>
#include <esp_chip_info.h>


static const uint32_t DESIRED_BIT_RATE = 500UL * 1000UL;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;
static uint32_t gCANSendDate = 0 ;

static ACAN_ESP32 & myCAN = ACAN_ESP32::can;
static uint32_t canCount = 0;

void canInit();
bool canSend(int id, uint8_t len, uint32_t interval, uint8_t *data);
CANMessage canReceive();

#endif // __ESP_CAN_H__