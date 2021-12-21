#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static void vSenderTask( void *pvParameters );

/* handle of Task. */
QueueHandle_t xQueue;


void app_main(void)
{
    printf("Hello world!\n");

    /* Create Queue */
    xQueue = xQueueCreate( 5, sizeof( int32_t ) );

    if(xQueue != NULL)
    {
        /* Create the thread(s) */
        xTaskCreate( vSenderTask, "Sender1", 1024, ( void * ) 100, 1, NULL );
    }
    else
    {
        printf("Create Queue failed\r\n");
    }

    int32_t lReceivedValue;
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 500UL );

    for(;;)
    {
	if( uxQueueMessagesWaiting( xQueue ) != 0 )
	{
	    printf( "Queue should have been empty!\r\n" );
	}

	xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );
	if( xStatus == pdPASS )
	{
	    /* Data was successfully received  */
	    printf( "Received = %d\r\n", lReceivedValue );
	}
	else
	{
	    printf( "Could not receive from the queue.\r\n" );
	}
    }
}

static void vSenderTask( void *pvParameters )
{
	int32_t lValueToSend;
	BaseType_t xStatus;

	lValueToSend = ( int32_t ) pvParameters;

	for(;;)
	{
	    xStatus = xQueueSendToBack( xQueue, &lValueToSend, 0 );
	    if(xStatus != pdPASS )
	    {
		printf("Could not send to the queue.\r\n");
	    }
	    else
	    {
		printf("Success send to the queue. \n");
	    }
	    
	    vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}
