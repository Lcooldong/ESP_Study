#include "esp_log.h"
#include "stdio.h"

static const char* TAG_test = "TEST";

void print_test(const char* text)
{
    ESP_LOGI(TAG_test, "%s", text);
}