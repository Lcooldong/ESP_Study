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
#include "uart_function.c"
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




void app_main(void)
{

	nvs_init();
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
	my_data->state = 0;
	my_data->magic = esp_random();
	my_data->count = CONFIG_ESPNOW_SEND_COUNT;		// 100
	my_data->delay = CONFIG_ESPNOW_SEND_DELAY;		// 1000 = 1s
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




//    my_data = malloc(sizeof(espnow_send_param_t));
//	memset(my_data, 0, sizeof(espnow_send_param_t));
//	if (my_data == NULL) {
//		ESP_LOGE(TAG, "Malloc send parameter fail");
//		vSemaphoreDelete(s_espnow_queue);
//		esp_now_deinit();
//	}
//
//    my_data->unicast = false;
//    my_data->broadcast = true;
//    my_data->state = 0;
//    my_data->magic = esp_random();
//    my_data->count = CONFIG_ESPNOW_SEND_COUNT;
//    my_data->delay = CONFIG_ESPNOW_SEND_DELAY;
//    my_data->len = CONFIG_ESPNOW_SEND_LEN;
//    my_data->buffer = malloc(CONFIG_ESPNOW_SEND_LEN);
//
////    s_evt_group = xEventGroupCreate();
////    assert(s_evt_group);
//    espnow_event_t evt;
//	uint8_t recv_state = 0;
//	uint16_t recv_seq = 0;
//	int recv_magic = 0;
//	bool is_broadcast = false;
//
//    free(my_data);
}
