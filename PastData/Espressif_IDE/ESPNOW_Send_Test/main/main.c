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
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "espnow_base_config.h"

// User include
#include "freertos/event_groups.h"
#include <driver/gpio.h>
#include <driver/uart.h>

// User Value
static EventGroupHandle_t s_evt_group;

const char *TAG = "espnow_sender";

/////////////////////////////////////////////////////////////////



uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint16_t s_espnow_seq[ESPNOW_DATA_MAX] = { 0, 0 };
xQueueHandle s_espnow_queue;



esp_err_t send_espnow_data(void){
	const uint8_t destination_mac[] = BROADCAST_MAC;
	static my_data_t data;

	// Go to the user function to populate the data to send
	my_data_populate(&data);

	// Send it
	ESP_LOGI(TAG, "Sending %u bytes to " MACSTR, sizeof(data), MAC2STR(destination_mac));
	esp_err_t err = esp_now_send(destination_mac, (uint8_t*)&data, sizeof(data));
	if(err != ESP_OK){
		ESP_LOGE(TAG, "Send error (%d)", err);
		return ESP_FAIL;
	}

	// Wait for callback function to set status bit
	EventBits_t bits = xEventGroupWaitBits(s_evt_group, BIT(ESP_NOW_SEND_SUCCESS) | BIT(ESP_NOW_SEND_FAIL), pdTRUE, pdFALSE, 2000 / portTICK_PERIOD_MS);
	if(!(bits & BIT(ESP_NOW_SEND_SUCCESS))){
		if (bits & BIT(ESP_NOW_SEND_FAIL)){
			ESP_LOGE(TAG, "Send error");
			return ESP_FAIL;
		}
		ESP_LOGE(TAG, "Send timed out");
		return ESP_ERR_TIMEOUT;
	}

	ESP_LOGI(TAG, "Sent!");
	return ESP_OK;


}

static void packet_sent_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    assert(status == ESP_NOW_SEND_SUCCESS || status == ESP_NOW_SEND_FAIL);
    xEventGroupSetBits(s_evt_group, BIT(status));
}

static void init_espnow_slave(void){
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(MY_ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start() );
#if MY_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(MY_ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(packet_sent_cb) );
    ESP_ERROR_CHECK( esp_now_set_pmk((const uint8_t *)MY_ESPNOW_PMK) );

    // Alter this if you want to specify the gateway mac, enable encyption, etc
    const esp_now_peer_info_t broadcast_destination = {
        .peer_addr = MY_RECEIVER_MAC,
        .channel = MY_ESPNOW_CHANNEL,
        .ifidx = MY_ESPNOW_WIFI_IF
    };
    ESP_ERROR_CHECK( esp_now_add_peer(&broadcast_destination) );
}









void app_main(void)
{
    // Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    wifi_init();
    espnow_init();


//    init_espnow_slave();
    ESP_LOGI(TAG, "Setup Done");
    vTaskDelay(2000/ portTICK_PERIOD_MS);
//    uint8_t destination_mac[] = BROADCAST_MAC;
//    my_data_t data;
//    data.button_pushed = 1;
//    data.random_value = 2;


//    // event 처리용 없으면 작동안함
//    s_evt_group = xEventGroupCreate();
//	assert(s_evt_group);
//
//    while(true)
//    {
//    	ESP_LOGI(TAG, "Send Begin");
//    	send_espnow_data();
////    	esp_now_send(destination_mac, (uint8_t*)&data, sizeof(data));
//    	vTaskDelay(1000/ portTICK_PERIOD_MS);
//
//    }


}
