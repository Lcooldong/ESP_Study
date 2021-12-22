#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#define mainONE_SHOT_TIMER_PERIOD	( pdMS_TO_TICKS( 3333UL ) )
#define mainAUTO_RELOAD_TIMER_PERIOD	( pdMS_TO_TICKS( 500UL ) )

/* The Software timer callback functions. */
static void prvTimerCallback( TimerHandle_t xTimer );

TimerHandle_t xAutoReloadTimer, xOneShotTimer;

void app_main(void)
{
	BaseType_t xTimer1Started, xTimer2Started;

	/* Create the one shot software timer */
	xOneShotTimer = xTimerCreate( "OneShot", mainONE_SHOT_TIMER_PERIOD, pdFALSE, 0,  prvTimerCallback);

	/* Create the auto-reload software timer */
	xAutoReloadTimer = xTimerCreate( "AutoReload", mainAUTO_RELOAD_TIMER_PERIOD, pdTRUE, 0,  prvTimerCallback);

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

static void prvTimerCallback( TimerHandle_t xTimer )
{
	TickType_t xTimeNow;
	uint32_t ulExecutionCount =0;

	/* Obtain the ID */
	ulExecutionCount = ( uint32_t ) pvTimerGetTimerID( xTimer );
	ulExecutionCount++;
	vTimerSetTimerID( xTimer, ( void * ) ulExecutionCount );

	/* Obtain the current tick count. */
	xTimeNow = xTaskGetTickCount();

   	if( xTimer == xOneShotTimer )
	{
		printf( "One-shot timer callback executing %d\r\n", xTimeNow );
	}
	else
	{
		printf( "Auto-reload timer callback executing %d\r\n", xTimeNow );

		if( ulExecutionCount == 5 )
		{
			xTimerStop( xTimer, 0 );
		}
	}
}
