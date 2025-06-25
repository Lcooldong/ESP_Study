#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
// #include "sdkconfig.h"
#include "stdio.h"


#define LED_BUILTIN GPIO_NUM_15


static const char *TAG = "BLINK";
static bool led_state = false;

void app_main() 
{

    gpio_reset_pin(LED_BUILTIN);
    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        led_state = !led_state;
        gpio_set_level(LED_BUILTIN, led_state);
        ESP_LOGI(TAG, "LED:%s", led_state ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(100));
        
    }
    
}