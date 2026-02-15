/*
 * uart_function.h
 *
 *  Created on: 2022. 6. 16.
 *      Author: s_coo
 */

#ifndef COMPONENTS_UART_INCLUDE_UART_FUNCTION_H_
#define COMPONENTS_UART_INCLUDE_UART_FUNCTION_H_

#include "esp_err.h"
#include "esp_now.h"

#define ESPNOW_MAXDELAY 512

// UART && GPIO
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define UART UART_NUM_2
#define BUILTIN_LED 2
static uint8_t s_example_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static const int RX_BUF_SIZE = 1024;
static const char *TAG = "espnow_example";
int num = 0;


#endif /* COMPONENTS_UART_INCLUDE_UART_FUNCTION_H_ */
