/*
 * uart_function.c
 *
 *  Created on: 2022. 6. 16.
 *      Author: s_coo
 */

#include "uart_function.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "string.h"


void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task(void *arg)
{
//    static const char *TX_TASK_TAG = "TX_TASK";
//    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

	char* Txdata = (char*) malloc(30);
    while (1) {
//        sendData(TX_TASK_TAG, "Hello world\r\n");
    	sprintf(Txdata, "Hello world %d\r\n", num++);
    	uart_write_bytes(UART, Txdata, strlen(Txdata));
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
    free(Txdata);
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    char* Txdata = (char*) malloc(30);
    while (1) {
        const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 500 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = '\0';
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
//            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

            if(data[0] == '1' && rxBytes == 1)
            {
            	 gpio_set_level(BUILTIN_LED, 1);
            	 sprintf(Txdata, "%d\r\n", num++);

				 if (esp_now_send(s_example_broadcast_mac, (uint8_t*)Txdata, sizeof(Txdata)) != ESP_OK)
				 {
					ESP_LOGE(TAG, "Send error");
					esp_now_deinit();
					vTaskDelete(NULL);
            	 }
				 else
				 {
					 ESP_LOGI(TAG, "Send Complete");
				 }
            }
            else if(data[0] == '0' && rxBytes == 1)
    		{
            	 gpio_set_level(BUILTIN_LED, 0);
    		}

        }

    }
    free(data);
    free(Txdata);
}

