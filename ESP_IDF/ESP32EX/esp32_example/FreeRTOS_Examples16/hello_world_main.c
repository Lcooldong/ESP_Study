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

#define mainBACKLIGHT_TIMER_PERIOD	( pdMS_TO_TICKS( 5000UL ) )

/* The Software timer callback functions. */
static void prvBacklightTimerCallback( TimerHandle_t xTimer );

uint8_t count_i;
static BaseType_t xSimulatedBacklightOn = pdFALSE;
static TimerHandle_t xBacklightTimer = NULL;

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

    xSimulatedBacklightOn = pdFALSE;
    xBacklightTimer = xTimerCreate( "Backlight", mainBACKLIGHT_TIMER_PERIOD, pdFALSE, 0, prvBacklightTimerCallback );
    /* Start the timer. */
    xTimerStart( xBacklightTimer, 0 );

    const TickType_t xShortDelay = pdMS_TO_TICKS( 500 );
    TickType_t xTimeNow;
    printf( "Press a key to turn the backlight on.\r\n" );

    for( ;; )
    {
	/* Has a key been pressed? */
	if(count_i != 0 )
	{
	    /* Record the time at which the key press was noted. */
	    xTimeNow = xTaskGetTickCount();

	    /* A key has been pressed. */
	    if( xSimulatedBacklightOn == pdFALSE )
	    {
		xSimulatedBacklightOn = pdTRUE;
		printf( "Key pressed, turning backlight ON at time %d\r\n", xTimeNow );
	    }
	    else
	    {
		printf( "Key pressed, resetting software timer at time %d\r\n", xTimeNow );
	    }
	    xTimerReset( xBacklightTimer, xShortDelay );
	}

	vTaskDelay( xShortDelay );
    }
}

static void prvBacklightTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow = xTaskGetTickCount();

    /* The backlight timer expired, turn the backlight off. */
    xSimulatedBacklightOn = pdFALSE;
    /* Print the time at which the backlight was turned off. */
    printf( "Timer expired, turning backlight OFF at time %d\r\n", xTimeNow );
}
