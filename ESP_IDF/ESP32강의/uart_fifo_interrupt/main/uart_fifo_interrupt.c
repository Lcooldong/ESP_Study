/* uart_fifo_interrupt Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/select.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_system.h"

#define rxBufferSIZE (256)
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

char UART_SlaveRxBuffer[rxBufferSIZE];
int cnt = 0;
bool trigger = false;

static void UART_init()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, rxBufferSIZE * 2, 0, 0, NULL, 0);
 }

static void IRAM_ATTR uart_intr_handle(void *arg)
{
    uint16_t rx_fifo_len = 0;

    rx_fifo_len = UART1.status.rxfifo_cnt; // read number of bytes in UART buffer

    while(rx_fifo_len)
    {
        char ch = UART1.fifo.rw_byte;

        if(ch == '\n' || cnt == rxBufferSIZE-1){
            UART_SlaveRxBuffer[cnt] = '\0';
            cnt = 0;
            trigger = true;
            break;
        }
        UART_SlaveRxBuffer[cnt] = ch; // read all bytes
        cnt++;
        rx_fifo_len--;        
    }
    uart_clear_intr_status(UART_NUM_1, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
}

void app_main()
{
    printf("ESP32 UART FIFO Interrupt Test Code\n");
    UART_init();
    memset(UART_SlaveRxBuffer, 0x0, sizeof(UART_SlaveRxBuffer));

    uart_isr_free(UART_NUM_1); 
    uart_isr_register(UART_NUM_1, uart_intr_handle, NULL, ESP_INTR_FLAG_IRAM, NULL);
    uart_enable_rx_intr(UART_NUM_1);

    while (1) 
    { 
        if(trigger)
        {
            printf("%s\n",UART_SlaveRxBuffer);
            trigger = false;
        }
        vTaskDelay(1);
    }
}