#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static void vSenderTask( void *pvParameters );

typedef enum
{
    eSender1,
    eSender2
} DataSource_t;

/* Define the structure type that will be passed on the queue. */
typedef struct
{
    uint8_t ucValue;
    DataSource_t eDataSource;
} Data_t;

/* Declare two variables of type Data_t that will be passed on the queue. */
static const Data_t xStructsToSend[ 2 ] =
{
    { 100, eSender1 }, /* Used by Sender1. */
    { 200, eSender2 }  /* Used by Sender2. */
};

/* handle of Task. */
QueueHandle_t xQueue;

void app_main(void)
{
    printf("Hello world!\n");

    /* Create Queue */
    xQueue = xQueueCreate( 3, sizeof( Data_t ) );

    if(xQueue != NULL)
    {
	/* Create the thread(s) */
	xTaskCreate( vSenderTask, "Sender1", 1024, ( void * ) &xStructsToSend[0], 2, NULL );
	xTaskCreate( vSenderTask, "Sender2", 1024, ( void * ) &xStructsToSend[1], 2, NULL );
    }
    else
    {
	printf("Create Queue failed\r\n");
    }


    BaseType_t xStatus;
    Data_t xReceivedStructure;

    for(;;)
    {
	if( uxQueueMessagesWaiting( xQueue ) != 3 )
	{
	    printf( "Queue should have been full!\r\n" );
	}

	xStatus = xQueueReceive( xQueue, &xReceivedStructure, 0 );

	if( xStatus == pdPASS )
	{
	    /* Data was successfully received  */
	    if( xReceivedStructure.eDataSource == eSender1 )
	    {
		printf( "From Sender 1 = %d\r\n", xReceivedStructure.ucValue );
	    }
	    else
	    {
		printf( "From Sender 2 = %d\r\n", xReceivedStructure.ucValue );
	    }
	}
	else
	{
	    printf( "Could not receive from the queue.\r\n" );
	}
    }
}

static void vSenderTask( void *pvParameters )
{
	BaseType_t xStatus;
	const TickType_t xTicksToWait = pdMS_TO_TICKS( 300UL );

	for(;;)
	{
	    xStatus = xQueueSendToBack( xQueue, pvParameters, xTicksToWait );

	    if(xStatus != pdPASS )
	    {
		printf("Could not send to the queue.\r\n");
	    }
	}
}
