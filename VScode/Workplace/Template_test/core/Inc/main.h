#pragma once

#define portTICK_PERIOD_MS 1000

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"


class Main final
{
public:
    esp_err_t setup(void);
    void loop(void);
};