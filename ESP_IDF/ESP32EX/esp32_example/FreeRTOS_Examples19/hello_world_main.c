
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int i = 0;

void vTask1(void *pvParameters)
{
    for(;;)
    {
        printf("VTask1\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void vTask2(void *pvParameters)
{
    int j;
    for(;;)
    {
        printf("VTask2\n");
	if(i == 5)
	{
	    vTaskSuspendAll();
	    for(j=0; j<0x2FFFFFF; j++);
	    xTaskResumeAll();
	    printf("Scheduling Start\n");
	}
	vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void vTask3(void *pvParameters)
{
    for(;;)
    {
        printf("VTask3\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    printf("Hello world!\n");

    xTaskCreatePinnedToCore(vTask1, "Task 1", 1024, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(vTask2, "Task 2", 1024, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(vTask3, "Task 3", 1024, NULL, 3, NULL, tskNO_AFFINITY);

    while(1)
    {
	printf("Restarting in %d seconds...\n", i);
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
