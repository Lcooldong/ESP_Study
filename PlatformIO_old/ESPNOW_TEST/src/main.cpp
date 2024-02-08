#include "test.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdio.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "led_strip.h"



extern "C"
{

    #define LED_PIN GPIO_NUM_2
    #define LED_COUNT 60
    #define CONFIG_BLINK_LED_RMT  1
    #define CONFIG_BLINK_LED_RMT_CHANNEL 0
    #define BLINK_GPIO 2
    
    static uint8_t s_led_state = 0;
    static const char* TAG = "MAIN";
    const char* test_text = "from test.cpp file";


    #ifdef CONFIG_BLINK_LED_RMT
    static led_strip_t *pStrip_a;
    static void blink_led(void)
    {
        /* If the addressable LED is enabled */
        if (s_led_state) {
            /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
            pStrip_a->set_pixel(pStrip_a, 0, 255, 40, 100);
            /* Refresh the strip to send data */
            pStrip_a->refresh(pStrip_a, 100);
        } else {
            /* Set all LED off to clear all pixels */
            pStrip_a->clear(pStrip_a, 50);
        }
    }

    static void configure_led(void)
    {
        ESP_LOGI(TAG, "Example configured to blink addressable LED!");
        /* LED strip initialization with the GPIO and pixels number*/
        pStrip_a = led_strip_init(CONFIG_BLINK_LED_RMT_CHANNEL, BLINK_GPIO, 1);
        /* Set all LED off to clear all pixels */
        pStrip_a->clear(pStrip_a, 50);
    }

    #endif



    void app_main() {

        configure_led();

        while(true){
            // ESP_LOGD(TAG,"--Hello World--");
            printf("Turning the LED %s!\r\n", s_led_state == true ? "ON" : "OFF");
            print_test(test_text);
            // ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
            blink_led();
            /* Toggle the LED state */
            s_led_state = !s_led_state;

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }


    }

}

