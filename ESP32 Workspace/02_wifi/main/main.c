#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BLINK_GPIO 10 

static const char *TAG = "BLINK";

void app_main(void) 
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    bool led_state = false;

    while (1) {
        led_state = !led_state;
        gpio_set_level(BLINK_GPIO, led_state);

        ESP_LOGI(TAG, "LED %s", led_state ? "ON" : "OFF");

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second
    }
} 

