#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char *pcTextForTask1 = "111111111111111111111111111111111111111111";
const char *pcTextForTask2 = "222222222222222222222222222222222222222222";

void vTask1(void *pvParameters)
{
    char *pcTaskName;
    pcTaskName = (char*)pvParameters;

    for(;;)
    {
        printf("%s\n",pcTaskName);
        //vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    int i = 0;

    printf("Hello world!\n");

    xTaskCreate(vTask1, "Task 1", 1024, (void*)pcTextForTask1, 3, NULL);
    xTaskCreate(vTask1, "Task 2", 1024, (void*)pcTextForTask2, 4, NULL);

    while(1)
    {
        printf("Restarting in %d seconds...\n", i);
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
