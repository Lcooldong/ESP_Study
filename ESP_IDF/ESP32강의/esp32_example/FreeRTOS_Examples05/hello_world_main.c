#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* handle of Task. */
TaskHandle_t xTask1Handle;

void vTask1(void *pvParameters)
{
    printf( "Task2 is running and delete\r\n" );
    vTaskDelete( xTask1Handle );
}

void app_main(void)
{
    printf("Hello world!\n");

    while(1)
    {
        printf("Task1 is running\r\n");
        xTaskCreate(vTask1, "Task 1", 1024, NULL, 3, &xTask1Handle);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
