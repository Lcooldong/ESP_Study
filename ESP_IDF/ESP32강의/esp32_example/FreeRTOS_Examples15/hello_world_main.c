#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"


#define GPIO_INPUT_USER_BUTTON		0 
#define GPIO_INPUT_BUTTON_SEL		(1ULL<<GPIO_INPUT_USER_BUTTON)
#define ESP_INTR_FLAG_DEFAULT		0

#define HEALTHY_PERIOD		( pdMS_TO_TICKS( 2000UL ) )
#define ERROR_PERIOD		( pdMS_TO_TICKS( 300UL ) )

/* The Software timer callback functions. */
static void prvTimerCallback( TimerHandle_t xTimer );

uint8_t count_i;
TimerHandle_t xAutoReloadTimer;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	count_i = (count_i+1)%2;
}

void app_main(void)
{
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_BUTTON_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_USER_BUTTON, gpio_isr_handler, (void*) GPIO_INPUT_USER_BUTTON);

    BaseType_t xTimerStarted;

    /* Create the auto-reload software timer */
    xAutoReloadTimer = xTimerCreate( "AutoReload", HEALTHY_PERIOD, pdTRUE, 0,  prvTimerCallback);
    /* Check the timers were created. */
    if( xAutoReloadTimer != NULL )
    {
	/* Start the software timers */
	xTimerStarted = xTimerStart( xAutoReloadTimer, 0 );
	if(  xTimerStarted == pdPASS )
	{
	    printf("Start Software timer\r\n");
	}
    }

    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void prvTimerCallback( TimerHandle_t xTimer )
{
	if(count_i)
	{
		xTimerChangePeriod(xAutoReloadTimer, ERROR_PERIOD, 0);
	}
	else
	{
		xTimerChangePeriod(xAutoReloadTimer, HEALTHY_PERIOD, 0);
	}

	printf("Timer Call \n");
}
