#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

void speed_task(void *pvParameters)
{
    ParamsThread_t *thread_params = (ParamsThread_t *)pvParameters;

    bool last = 1;

    while (1) {
        bool now = gpio_get_level(SPEED_PIN);

	// Lock the mutex
	xSemaphoreTake( thread_params->my_mutex, portMAX_DELAY );

        if (now == 0 && last == 1) {
            thread_params->my_params->speed = (thread_params->my_params->speed + 100 > 500) ? 100 :  thread_params->my_params->speed + 100;
            ESP_LOGI("SPEED", "Speed=%d", thread_params->my_params->speed);
            thread_params->my_params->led_update_needed = true;
        }

	// Unlock the mutex
	xSemaphoreGive( thread_params->my_mutex );

        last = now;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}