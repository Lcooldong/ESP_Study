/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO
#define GPIO_OUTPUT_IO_0 2
#define GPIO_OUTPUT_PIN_SEL 1ULL<<GPIO_OUTPUT_IO_0  //1ULL : unsigned long long

void app_main(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    // struct config
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    int cnt = 0;
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        cnt++;
        if(cnt % 2){
            printf("Turning off the LED: %d\n", cnt);
            gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
        }else{
            printf("Turning on  the LED: %d\n", cnt);
            gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        //printf("Turning off the LED\n");
        //gpio_set_level(BLINK_GPIO, 0);
        //gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
        
        /* Blink on (output high) */
        //printf("Turning on the LED\n");
        //gpio_set_level(BLINK_GPIO, 1);
        //gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
