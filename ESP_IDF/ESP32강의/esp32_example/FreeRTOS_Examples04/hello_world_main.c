#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* handle of Task. */
TaskHandle_t xTask2Handle;

void vTask1(void *pvParameters)
{
    UBaseType_t uxPriority;
    uxPriority = uxTaskPriorityGet( NULL );

    for(;;)
    {
        printf("Task1 is running\r\n");

	/* Setting the Task2 priority */
	printf("About to raise the Task2 priority\r\n");
	vTaskPrioritySet( xTask2Handle, ( uxPriority + 1 ) );
    }
}

void vTask2(void *pvParameters)
{
    UBaseType_t uxPriority;
    uxPriority = uxTaskPriorityGet( NULL );

    for(;;)
    {
	printf("Task2 is running\r\n");

	/* Setting the Task2 priority */
	printf("About to lower the Task2 priority\r\n");
	vTaskPrioritySet( NULL, ( uxPriority - 2 ) );
    }
}


void app_main(void)
{
    printf("Hello world!\n");

    xTaskCreate(vTask1, "Task 1", 1024, NULL, 4, NULL);
    xTaskCreate(vTask2, "Task 2", 1024, NULL, 3, &xTask2Handle);

    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
