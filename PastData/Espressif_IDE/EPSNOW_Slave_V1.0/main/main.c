/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
   This example shows how to use ESPNOW.
   Prepare two device, one for sending ESPNOW data and another for receiving
   ESPNOW data.
*/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <esp_basic_config.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "uart_function.h"
#include "espnow_function.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/task.h"


const char *TAG = "espnow_uart_master";
uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t serial_read = 0;

void my_data_populate(my_data_t *data)
{
	ESP_LOGI(TAG, "Populating my_data_t");
	memcpy(data->mac_addr, s_broadcast_mac, ESP_NOW_ETH_ALEN);
	data->button_pushed = 0;
}

static esp_err_t uart_espnow(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* uart_data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    static my_data_t data;

    while(1){
    	const int rxBytes = uart_read_bytes(UART, uart_data, RX_BUF_SIZE, 500 / portTICK_RATE_MS);
		if (rxBytes > 0) {
			uart_data[rxBytes] = '\0';
		    ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, uart_data);

		    my_data_populate(&data);

		    if(uart_data[0] == '2' && rxBytes == 1)
		    {
			  gpio_set_level(BLINK_GPIO, 1);
//			  my_data_t *send_param = (my_data_t *)arg;

			  esp_err_t err = esp_now_send(data.mac_addr, (uint8_t*)&data, sizeof(data));
			  if (err != ESP_OK) {
				  ESP_LOGE(TAG, "Send error");
//				  espnow_deinit(send_param);
//				  vTaskDelete(NULL);
				  return ESP_FAIL;
			  }
		    }
		    else if(uart_data[0] == '0' && rxBytes == 1)
		    {
			  gpio_set_level(BLINK_GPIO, 0);
		    }
		}
    }
    free(uart_data);
    return ESP_OK;
}




void app_main(void)
{
	uart_init();
	gpio_reset_pin(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );


    wifi_init();
    espnow_init();


    xTaskCreate((void *)uart_espnow, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);		// higher priority
//	xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}
