/*
 * uart_function.h
 *
 *  Created on: 2022. 6. 16.
 *      Author: s_coo
 */

#ifndef COMPONENTS_UART_INCLUDE_UART_FUNCTION_H_
#define COMPONENTS_UART_INCLUDE_UART_FUNCTION_H_

#include "sdkconfig.h"
#include "stdint.h"

static const int RX_BUF_SIZE = 1024;

//#define TXD_PIN (GPIO_NUM_17)
//#define RXD_PIN (GPIO_NUM_16)
//#define UART UART_NUM_2
#define BLINK_GPIO 2

#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_20)
#define UART UART_NUM_0

typedef struct __attribute__((packed)){
	uint8_t STX;
	uint8_t ETX;
	uint8_t len;
	uint8_t data;
}uart_data_t;



// function
void uart_init(void);
int sendData(const char* logName, const char* data);
void tx_task(void *arg);
void rx_task(void *arg);


#endif /* COMPONENTS_UART_INCLUDE_UART_FUNCTION_H_ */
