/*
 * espnow_function.c
 *
 *  Created on: 2022. 6. 26.
 *      Author: user
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

#include "freertos/task.h"
#include "espnow_basic_config.h"

espnow_send_param_t *my_data2;
extern const char *TAG;
extern xQueueHandle s_espnow_queue;
extern uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN];
extern uint16_t s_espnow_seq[ESPNOW_DATA_MAX];
extern bool btn_flag;
bool pairing_flag = 0;



//EventGroupHandle_t s_evt_group;

/* WiFi should start before using ESPNOW */
void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
}

esp_err_t espnow_init(void)
{

    s_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_event_t));
    if (s_espnow_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_recv_cb) );

    /* Set primary master key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(s_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, s_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    return ESP_OK;
}

esp_err_t broadcast_init(espnow_send_param_t *target_param)
{
	espnow_send_param_t *send_param = target_param;

	/* Initialize sending parameters. */
	send_param = malloc(sizeof(espnow_send_param_t));
	memset(send_param, 0, sizeof(espnow_send_param_t));
	if (send_param == NULL) {
		ESP_LOGE(TAG, "Malloc send parameter fail");
		vSemaphoreDelete(s_espnow_queue);
		esp_now_deinit();
		return ESP_FAIL;
	}else{
		ESP_LOGI(TAG, "broadcast initialize");
	}

	send_param->unicast = false;
	send_param->broadcast = true;
	send_param->state = 0;
	send_param->magic = esp_random();
	send_param->count = CONFIG_ESPNOW_SEND_COUNT;		// 100
	send_param->delay = CONFIG_ESPNOW_SEND_DELAY;		// 1000 = 1s
	send_param->len = CONFIG_ESPNOW_SEND_LEN;			// 10
	send_param->buffer = malloc(CONFIG_ESPNOW_SEND_LEN);
	if (send_param->buffer == NULL) {
		ESP_LOGE(TAG, "Malloc send buffer fail");
		free(send_param);
		vSemaphoreDelete(s_espnow_queue);
		esp_now_deinit();
		return ESP_FAIL;
	}
	memcpy(send_param->dest_mac, s_broadcast_mac, ESP_NOW_ETH_ALEN);
	espnow_data_prepare(send_param);

//	xTaskCreate(espnow_task, "espnow_task", 2048, send_param, 4, NULL);

	return ESP_OK;
}

void espnow_deinit(espnow_send_param_t *send_param)
{
    free(send_param->buffer);
    free(send_param);
    vSemaphoreDelete(s_espnow_queue);
    esp_now_deinit();
}


/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    espnow_event_t evt;
    espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }else{
    	printf("Send callback working\r\n");
    }

    evt.id = ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    espnow_event_t evt;
    espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }else{
    	printf("------Receiving Completed------\r\n");
    	ESP_LOGI(TAG, "Received Callback");
    }

    evt.id = ESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }


    espnow_data_t *buf = (espnow_data_t *)recv_cb->data;
	for(int i=0; i<ESP_NOW_ETH_ALEN; i++)
	{
		printf("%02x", recv_cb->mac_addr[i]);
		if( i < ESP_NOW_ETH_ALEN-1){
			printf(":");
		}
		else
		{
			printf("\r\n");
		}
	}
    printf("%d, %d, %d, %ld, %d, %d, %d\r\n", buf->type, buf->state ,buf->seq_num, buf->magic, buf->crc, buf->payload[0], recv_cb->data_len );

    if (btn_flag == 1)
    {
    	my_data2 = malloc(sizeof(espnow_send_param_t));
    	memset(my_data2, 0, sizeof(espnow_send_param_t));
    	if (my_data2 == NULL) {
    		ESP_LOGE(TAG, "Malloc send parameter fail");
    		vSemaphoreDelete(s_espnow_queue);
    		esp_now_deinit();

    	}else{
    		ESP_LOGI(TAG, "prepare for return");
    	}

    	my_data2->unicast = false;						// 1 : 1 comunication
    	my_data2->broadcast = true;
    	my_data2->state = buf->state + 1;
    	my_data2->magic = esp_random();
    	my_data2->count = CONFIG_ESPNOW_SEND_COUNT;		// 100
    	my_data2->delay = CONFIG_ESPNOW_SEND_DELAY;		// 1000 = 1s
    	my_data2->len = CONFIG_ESPNOW_SEND_LEN;			// 10
    	my_data2->buffer = malloc(CONFIG_ESPNOW_SEND_LEN);
    	if (my_data2->buffer == NULL) {
    		ESP_LOGE(TAG, "Malloc send buffer fail");
    		free(my_data2);
    		vSemaphoreDelete(s_espnow_queue);
    		esp_now_deinit();

    	}
    	memcpy(my_data2->dest_mac, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
    	espnow_data_prepare(my_data2);


    	printf("buf-state : %d\r\n", my_data2->state);

		if (esp_now_send(recv_cb->mac_addr, my_data2->buffer, my_data2->len) != ESP_OK) {
			ESP_LOGE(TAG, "Send error");
			espnow_deinit(my_data2);
			vTaskDelete(NULL);
			btn_flag = 0;
		}else{
			btn_flag = 1;
			printf("-------Send Okay------\r\n");
		}
//		vTaskDelay(500/ portTICK_RATE_MS);



    }

}

/* Parse received ESPNOW data. */
int espnow_data_parse(uint8_t *data, uint16_t data_len, uint8_t *state, uint16_t *seq, int *magic)
{
    espnow_data_t *buf = (espnow_data_t *)data;
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(espnow_data_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return -1;
    }

    *state = buf->state;
    *seq = buf->seq_num;
    *magic = buf->magic;
    crc = buf->crc;
    buf->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

    if (crc_cal == crc) {
        return buf->type;
    }

    return -1;
}

/* Prepare ESPNOW data to be sent. */
void espnow_data_prepare(espnow_send_param_t *send_param)
{
    espnow_data_t *buf = (espnow_data_t *)send_param->buffer;

    assert(send_param->len >= sizeof(espnow_data_t));

    buf->type = IS_BROADCAST_ADDR(send_param->dest_mac) ? ESPNOW_DATA_BROADCAST : ESPNOW_DATA_UNICAST;
    buf->state = send_param->state;
    buf->seq_num = s_espnow_seq[buf->type]++;
    buf->crc = 0;
    buf->magic = send_param->magic;
    /* Fill all remaining bytes after the data with random values */
    esp_fill_random(buf->payload, send_param->len - sizeof(espnow_data_t));
    buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}

void espnow_task(void *pvParameter)
{
    espnow_event_t evt;
    uint8_t recv_state = 0;
    uint16_t recv_seq = 0;
    int recv_magic = 0;
    bool is_broadcast = false;
    int ret;

    vTaskDelay(2000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "Start sending broadcast data");

    /* Start sending broadcast ESPNOW data. */
    espnow_send_param_t *send_param = (espnow_send_param_t *)pvParameter;
    if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
        espnow_deinit(send_param);
        vTaskDelete(NULL);
    }

    while (xQueueReceive(s_espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
        switch (evt.id) {
            case ESPNOW_SEND_CB:
            {
                espnow_event_send_cb_t *send_cb = &evt.info.send_cb;
                is_broadcast = IS_BROADCAST_ADDR(send_cb->mac_addr);

                ESP_LOGD(TAG, "Send data to "MACSTR", status1: %d", MAC2STR(send_cb->mac_addr), send_cb->status);

                if (is_broadcast && (send_param->broadcast == false)) {
                    break;
                }

//                if (!is_broadcast) {
//                    send_param->count--;
//                    if (send_param->count == 0) {
//                        ESP_LOGI(TAG, "Send done");
//                        espnow_deinit(send_param);
//                        vTaskDelete(NULL);
//                    }
//                }

                /* Delay a while before sending the next data. */
                if (send_param->delay > 0) {
                    vTaskDelay(send_param->delay/portTICK_RATE_MS);
                }

                ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(send_cb->mac_addr));

                memcpy(send_param->dest_mac, send_cb->mac_addr, ESP_NOW_ETH_ALEN);
                espnow_data_prepare(send_param);

                /* Send the next data after the previous data is sent. */
                if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                    ESP_LOGE(TAG, "Send error");
                    espnow_deinit(send_param);
                    vTaskDelete(NULL);
                }
                break;
            }
            case ESPNOW_RECV_CB:
            {
                espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

                ret = espnow_data_parse(recv_cb->data, recv_cb->data_len, &recv_state, &recv_seq, &recv_magic);
                free(recv_cb->data);
                if (ret == ESPNOW_DATA_BROADCAST) {
                    ESP_LOGI(TAG, "Receive %dth broadcast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    /* If MAC address does not exist in peer list, add it to peer list. */
                    if (esp_now_is_peer_exist(recv_cb->mac_addr) == false) {
                        esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
                        if (peer == NULL) {
                            ESP_LOGE(TAG, "Malloc peer information fail");
                            espnow_deinit(send_param);
                            vTaskDelete(NULL);
                        }
                        memset(peer, 0, sizeof(esp_now_peer_info_t));
                        peer->channel = CONFIG_ESPNOW_CHANNEL;
                        peer->ifidx = ESPNOW_WIFI_IF;
                        peer->encrypt = true;
                        memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
                        memcpy(peer->peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
                        free(peer);
                    }

                    /* Indicates that the device has received broadcast ESPNOW data. */
                    if (send_param->state == 0) {
                        send_param->state = 1;
                    }

                    /* If receive broadcast ESPNOW data which indicates that the other device has received
                     * broadcast ESPNOW data and the local magic number is bigger than that in the received
                     * broadcast ESPNOW data, stop sending broadcast ESPNOW data and start sending unicast
                     * ESPNOW data.
                     */
                    if (recv_state == 1) {
                        /* The device which has the bigger magic number sends ESPNOW data, the other one
                         * receives ESPNOW data.
                         */
                        if (send_param->unicast == false && send_param->magic >= recv_magic) {
                    	    ESP_LOGI(TAG, "Start sending unicast data");
                    	    ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(recv_cb->mac_addr));

                    	    /* Start sending unicast ESPNOW data. */
                            memcpy(send_param->dest_mac, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                            espnow_data_prepare(send_param);
                            if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                                ESP_LOGE(TAG, "Send error");
                                espnow_deinit(send_param);
                                vTaskDelete(NULL);
                            }
                            else {
                                send_param->broadcast = false;
                                send_param->unicast = true;
                            }
                        }
                    }
                }
                else if (ret == ESPNOW_DATA_UNICAST) {
                    ESP_LOGI(TAG, "Receive %dth unicast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    /* If receive unicast ESPNOW data, also stop sending broadcast ESPNOW data. */
                    send_param->broadcast = false;
                }
                else {
                    ESP_LOGI(TAG, "Receive error data from: "MACSTR"", MAC2STR(recv_cb->mac_addr));
                }
                break;
            }
            default:
                ESP_LOGE(TAG, "Callback type error: %d", evt.id);
                break;
        }
    }
}


void received_queue_task(void *pvParameter)
{
	espnow_event_t evt;
	uint8_t recv_state = 0;
	uint16_t recv_seq = 0;
	int recv_magic = 0;
	bool is_broadcast = false;
	int ret;

	vTaskDelay(1000 / portTICK_RATE_MS);
	ESP_LOGI(TAG, "Queue Task");

	/* Start sending broadcast ESPNOW data. */
	espnow_send_param_t *send_param = (espnow_send_param_t *)pvParameter;


	while (xQueueReceive(s_espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
		switch (evt.id) {
			case ESPNOW_SEND_CB:
			{
				espnow_event_send_cb_t *send_cb = &evt.info.send_cb;
				is_broadcast = IS_BROADCAST_ADDR(send_cb->mac_addr);

				ESP_LOGD(TAG, "Send data to "MACSTR", status1: %d", MAC2STR(send_cb->mac_addr), send_cb->status);

				if (is_broadcast && (send_param->broadcast == false)) {
					break;
				}

				// Stop Sending to target
//				if (!is_broadcast) {
//					send_param->count--;
//					if (send_param->count == 0) {
//						ESP_LOGI(TAG, "Send done");
//						espnow_deinit(send_param);
//						vTaskDelete(NULL);
//					}
//				}

				/* Delay a while before sending the next data. */
				if (send_param->delay > 0) {
					vTaskDelay(send_param->delay/portTICK_RATE_MS);
				}

				ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(send_cb->mac_addr));

				memcpy(send_param->dest_mac, send_cb->mac_addr, ESP_NOW_ETH_ALEN);
				espnow_data_prepare(send_param);

				/* Send the next data after the previous data is sent. */
//				if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
//					ESP_LOGE(TAG, "Send error");
//					espnow_deinit(send_param);
//					vTaskDelete(NULL);
//				}
				break;
			}
			case ESPNOW_RECV_CB:
			{
				espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

				ret = espnow_data_parse(recv_cb->data, recv_cb->data_len, &recv_state, &recv_seq, &recv_magic);
				free(recv_cb->data);
				if (ret == ESPNOW_DATA_BROADCAST) {
					ESP_LOGI(TAG, "Receive %dth broadcast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);
//					ESP_LOGI(TAG, "%d", recv_cb->count);
					/* If MAC address does not exist in peer list, add it to peer list. */
					if (esp_now_is_peer_exist(recv_cb->mac_addr) == false) {
						esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
						if (peer == NULL) {
							ESP_LOGE(TAG, "Malloc peer information fail");
							espnow_deinit(send_param);
							vTaskDelete(NULL);
						}
						memset(peer, 0, sizeof(esp_now_peer_info_t));
						peer->channel = CONFIG_ESPNOW_CHANNEL;
						peer->ifidx = ESPNOW_WIFI_IF;
						peer->encrypt = true;
						memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
						memcpy(peer->peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
						ESP_ERROR_CHECK( esp_now_add_peer(peer) );
						free(peer);
					}

					/* Indicates that the device has received broadcast ESPNOW data. */
					if (send_param->state == 0) {
						send_param->state = 1;
					}

					/* If receive broadcast ESPNOW data which indicates that the other device has received
					 * broadcast ESPNOW data and the local magic number is bigger than that in the received
					 * broadcast ESPNOW data, stop sending broadcast ESPNOW data and start sending unicast
					 * ESPNOW data.
					 */
					if (recv_state == 1) {
						/* The device which has the bigger magic number sends ESPNOW data, the other one
						 * receives ESPNOW data.
						 */
						if (send_param->unicast == false && send_param->magic >= recv_magic) {
							ESP_LOGI(TAG, "Start sending unicast data");
							ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(recv_cb->mac_addr));

							/* Start sending unicast ESPNOW data. */
							memcpy(send_param->dest_mac, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
							espnow_data_prepare(send_param);
							if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
								ESP_LOGE(TAG, "Send error");
								espnow_deinit(send_param);
								vTaskDelete(NULL);
							}
							else {
								send_param->broadcast = false;
								send_param->unicast = true;
							}
						}
					}
				}
				else if (ret == ESPNOW_DATA_UNICAST) {
					ESP_LOGI(TAG, "Receive %dth unicast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

					/* If receive unicast ESPNOW data, also stop sending broadcast ESPNOW data. */
					send_param->broadcast = false;
				}
				else {
					ESP_LOGI(TAG, "Receive error data from: "MACSTR"", MAC2STR(recv_cb->mac_addr));
				}
				break;
			}
			default:
				ESP_LOGE(TAG, "Callback type error: %d", evt.id);
				break;
		}
	}
}


void espnow_return_task(void *pvParameter)
{
//    espnow_event_t evt;
//    uint8_t recv_state = 0;
//    uint16_t recv_seq = 0;
//    int recv_magic = 0;
//    bool is_broadcast = false;
//    int ret;

    vTaskDelay(2000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "Start sending return data");

    /* Start sending broadcast ESPNOW data. */
    for(int i=0; i < 5; i++)
    {
        espnow_send_param_t *send_param = (espnow_send_param_t *)pvParameter;
        if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
            ESP_LOGE(TAG, "Send error");
            espnow_deinit(send_param);
            vTaskDelete(NULL);
        }
        vTaskDelay(500/ portTICK_RATE_MS);
    }


}

