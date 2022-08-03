#include "main.h"

#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"


#define LOG_TAG "MAIN"

static Main my_main;

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(my_main.setup()); 


    while (true)
    {
        my_main.loop();     
    }
    
    
}

esp_err_t Main::setup(void)
{
    esp_err_t status {ESP_OK};

    ESP_LOGI(LOG_TAG, "Setup MAIN");
    vTaskDelay(3000/portTICK_PERIOD_MS);

    return status;
}

void Main::loop(void)
{
    ESP_LOGI(LOG_TAG, "Hello World");
    vTaskDelay(1000/portTICK_PERIOD_MS); 
}