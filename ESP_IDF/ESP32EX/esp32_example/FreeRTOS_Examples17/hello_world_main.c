#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Definitions for the event bits in the event group. */
#define mainFIRST_TASK_BIT	( 1UL << 0UL ) /* Event bit 0, which is set by a task. */
#define mainSECOND_TASK_BIT	( 1UL << 1UL ) /* Event bit 1, which is set by a task. */

/* task function */
static void vEventBitSettingTask( void *pvParameters );

/* Declare the event group */
EventGroupHandle_t xEventGroup;


void app_main(void)
{
    /* Create event group */
    xEventGroup = xEventGroupCreate();

    xTaskCreate( vEventBitSettingTask, "BitSetter", 1024, NULL, 1, NULL );

    const EventBits_t xBitsToWaitFor = (mainFIRST_TASK_BIT | mainSECOND_TASK_BIT);
    EventBits_t xEventGroupValue;

    for( ;; )
    {
	/* Block to wait for event bits to become set within the event group. */
	xEventGroupValue = xEventGroupWaitBits(
			xEventGroup,	// The event group to read.
			xBitsToWaitFor,	// Bits to test.
			pdTRUE,			// Clear bits on exit if the unblock condition is met.
			pdFALSE,		// Don't wait for all bits.
			portMAX_DELAY	// Don't time out.
										    );

	/* Print a message for each bit that was set. */
	if( ( xEventGroupValue & mainFIRST_TASK_BIT ) != 0 )
	{
	    printf( "Bit reading task -\t event bit 0 was set\r\n" );
	}

	if( ( xEventGroupValue & mainSECOND_TASK_BIT ) != 0 )
	{
	    printf( "Bit reading task -\t event bit 1 was set\r\n" );
	}
    }
}

static void vEventBitSettingTask( void *pvParameters )
{
    const TickType_t xDelay200ms = pdMS_TO_TICKS( 200UL );

    for( ;; )
    {
	vTaskDelay( xDelay200ms );
	printf( "Bit setting task -\t about to set bit 0.\r\n" );
	/* set event bit 0. */
	xEventGroupSetBits( xEventGroup, mainFIRST_TASK_BIT );

	vTaskDelay( xDelay200ms );
	printf( "Bit setting task -\t about to set bit 1.\r\n" );
	/* set event bit 1. */
	xEventGroupSetBits( xEventGroup, mainSECOND_TASK_BIT );
    }
}
