#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static void vUpdateData( void *pvParameters );

typedef struct xExampleStructure
{
    TickType_t xTimeStamp;
    uint32_t ulValue;
} Example_t;

/* handle of queue. */
static QueueHandle_t xMailbox;

void app_main(void)
{
    printf("Hello world!\n");

    /* Create Queue */
    xMailbox = xQueueCreate( 1, sizeof( Example_t ) );

    /* Create the tasks that send to the queues. */
    xTaskCreate( vUpdateData, "Update", 1024, NULL, 1, NULL );

    Example_t xData;
    TickType_t xPrevTimeStamp;
    const TickType_t xBlockTime = pdMS_TO_TICKS( 100 );

    memset(&xData, 0, sizeof(Example_t));

    for(;;)
    {
	xPrevTimeStamp = xData.xTimeStamp;
	xQueuePeek(xMailbox, &xData, portMAX_DELAY);

	if(xData.xTimeStamp > xPrevTimeStamp)
	{
	    printf("value: %d\r\n", xData.ulValue);
	}
	else
	{
	    printf("Not receive Mailbox\r\n");
	}

	vTaskDelay( xBlockTime );
    }
}

static void vUpdateData( void *pvParameters )
{
    Example_t xData;
    uint32_t value = 0;
    const TickType_t xBlockTime = pdMS_TO_TICKS( 500 );

    for(;;)
    {
	xData.ulValue = value++;
	xData.xTimeStamp = xTaskGetTickCount();

	xQueueOverwrite(xMailbox, &xData);

	vTaskDelay( xBlockTime );
    }
}
