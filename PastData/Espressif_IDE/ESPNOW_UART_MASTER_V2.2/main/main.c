//#define UART UART_NUM_2
//#define BLINK_GPIO 2


#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "espnow_basic_config.h"
#include "uart_function.h"
#include "driver/uart.h"
#include "driver/gpio.h"

//#include "freertos/event_groups.h"

#include "sdkconfig.h"

const char *TAG = "ESPNOW_UART_MASTER";
xQueueHandle s_espnow_queue;
uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint16_t s_espnow_seq[ESPNOW_DATA_MAX] = { 0, 0 };

espnow_send_param_t *my_data;
TaskHandle_t xHandle = NULL;


void nvs_init(){
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK( nvs_flash_erase() );
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );
}

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
			  vTaskSuspend( xHandle );	//  vTaskResume(TaskHandle_t xTaskToResume)
//			  my_data_t *send_param = (my_data_t *)arg;

			  esp_err_t err = esp_now_send(data.mac_addr, (uint8_t*)&data, sizeof(data));
			  if (err != ESP_OK) {
				  ESP_LOGE(TAG, "Send error");
//				  espnow_deinit(send_param);
//				  vTaskDelete(NULL);
				  return ESP_FAIL;
			  }else{
				  ESP_LOGI(TAG, "Send to Target");
			  }
		    }
		    else if(uart_data[0] == '0' && rxBytes == 1)
		    {
			  gpio_set_level(BLINK_GPIO, 0);
//			  vTaskResume(xHandle);
		    }else if(uart_data[0]== '3' && rxBytes == 1){



		    }
		}
    }
    free(uart_data);
    return ESP_OK;
}


void app_main(void)
{

	nvs_init();
	uart_init();
    wifi_init();
    espnow_init();
//    broadcast_init(my_data);

    /* Initialize sending parameters. */
    my_data = malloc(sizeof(espnow_send_param_t));
	memset(my_data, 0, sizeof(espnow_send_param_t));
	if (my_data == NULL) {
		ESP_LOGE(TAG, "Malloc send parameter fail");
		vSemaphoreDelete(s_espnow_queue);
		esp_now_deinit();

	}else{
		ESP_LOGI(TAG, "broadcast initialize");
	}

	my_data->unicast = false;
	my_data->broadcast = true;
	my_data->state = 1;
	my_data->magic = esp_random();
	my_data->count = 100;		// 100
	my_data->delay = CONFIG_ESPNOW_SEND_DELAY + 1000;		// 1000 = 1s
	my_data->len = CONFIG_ESPNOW_SEND_LEN;			// 10
	my_data->buffer = malloc(CONFIG_ESPNOW_SEND_LEN);
	if (my_data->buffer == NULL) {
		ESP_LOGE(TAG, "Malloc send buffer fail");
		free(my_data);
		vSemaphoreDelete(s_espnow_queue);
		esp_now_deinit();

	}
	memcpy(my_data->dest_mac, s_broadcast_mac, ESP_NOW_ETH_ALEN);
	espnow_data_prepare(my_data);


    ESP_LOGI(TAG, "Setup Done");
	vTaskDelay(1000/ portTICK_PERIOD_MS);

	xTaskCreate(espnow_task, "espnow_task", 2048, my_data, 4, &xHandle);
	xTaskCreate((void *)uart_espnow, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);		// higher priority
//		xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);

}
