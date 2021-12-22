#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"


#define mainONE_SHOT_TIMER_PERIOD	( pdMS_TO_TICKS( 3333UL ) )
#define mainAUTO_RELOAD_TIMER_PERIOD	( pdMS_TO_TICKS( 500UL ) )

/* The Software timer callback functions. */
static void prvOneShotTimerCallback( TimerHandle_t xTimer );
static void prvAutoReloadTimerCallback( TimerHandle_t xTimer );


void app_main(void)
{
    TimerHandle_t xAutoReloadTimer, xOneShotTimer;
    BaseType_t xTimer1Started, xTimer2Started;

    /* Create the one shot software timer */
    xOneShotTimer = xTimerCreate( "OneShot", mainONE_SHOT_TIMER_PERIOD, pdFALSE, 0, prvOneShotTimerCallback );

    /* Create the auto-reload software timer */
    xAutoReloadTimer = xTimerCreate( "AutoReload", mainAUTO_RELOAD_TIMER_PERIOD, pdTRUE, 0, prvAutoReloadTimerCallback );

    /* Check the timers were created. */
    if( ( xOneShotTimer != NULL ) && ( xAutoReloadTimer != NULL ) )
    {
	/* Start the software timers */
	xTimer1Started = xTimerStart( xOneShotTimer, 0 );
	xTimer2Started = xTimerStart( xAutoReloadTimer, 0 );

	if( ( xTimer1Started == pdPASS ) && ( xTimer2Started == pdPASS ) )
	{
	    printf("Start Software timer\r\n");
	}
    }

    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void prvOneShotTimerCallback( TimerHandle_t xTimer )
{
	static TickType_t xTimeNow;

	/* Obtain the current tick count. */
	xTimeNow = xTaskGetTickCount();

	printf( "One-shot timer callback executing %d\r\n", xTimeNow );
}

static void prvAutoReloadTimerCallback( TimerHandle_t xTimer )
{
	static TickType_t xTimeNow;

	/* Obtain the current tick count. */
	xTimeNow = xTaskGetTickCount();

	printf( "Auto-reload timer callback executing %d\r\n", xTimeNow );
}
