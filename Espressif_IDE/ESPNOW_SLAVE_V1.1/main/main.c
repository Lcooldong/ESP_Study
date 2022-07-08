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
#include "driver/uart.h"
#include "driver/gpio.h"

//#include "freertos/event_groups.h"

#include "sdkconfig.h"
#define BTN_PIN 3
static int count = 0;
bool btn_flag = false;

const char *TAG = "ESPNOW_SLAVE";
xQueueHandle s_espnow_queue;
uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint16_t s_espnow_seq[ESPNOW_DATA_MAX] = { 0, 0 };

espnow_send_param_t *my_data;
TaskHandle_t xHandle = NULL;
TaskHandle_t xHandle_return = NULL;


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


void app_main(void)
{

	nvs_init();
	uart_init();
    wifi_init();
    espnow_init();

    gpio_set_direction(BTN_PIN, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BTN_PIN, GPIO_PULLUP_ONLY);
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

	my_data->unicast = false;						// 1 : 1 comunication
	my_data->broadcast = true;
	my_data->state = 1;
	my_data->magic = esp_random();
	my_data->count = 100;		// Send count
	my_data->delay = 0;		// 1000 = 1s
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

	xTaskCreate(received_queue_task, "espnow_task", 2048, my_data, 4, &xHandle);


//	esp_now_unregister_recv_cb();
	while(true)
	{
		while(btn_flag == 0)
		{
			if(gpio_get_level(BTN_PIN) == 0)
			{

				vTaskDelay(5 / portTICK_RATE_MS);
				printf("Button Pressed %d\r\n", count);
				count++;
				if(count >= 100)
				{
					ESP_LOGI(TAG, "STOP COUNT");
					btn_flag = 1;
					break;
				}

			}
			else
			{
				count = 0;
			}
			vTaskDelay(20 / portTICK_RATE_MS);
		}
		vTaskDelay(50/ portTICK_RATE_MS);

	}
}
