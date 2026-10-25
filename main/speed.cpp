#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

void speed_listener(void *pvParameters)
{
    ParamsThread_t *thread_params = (ParamsThread_t *)pvParameters;

    bool last = 1;

    while (1) {
        bool now = gpio_get_level(SPEED_PIN);

        if (now == 0 && last == 1) {
            // Lock the mutex
	        xSemaphoreTake( thread_params->my_mutex, portMAX_DELAY );

            thread_params->my_params->speed = (thread_params->my_params->speed + 100 > 500) ? 100 :  thread_params->my_params->speed + 100;
            ESP_LOGI("SPEED", "Speed=%d", thread_params->my_params->speed);

            // Unlock the mutex
	        xSemaphoreGive( thread_params->my_mutex );
            xTaskNotifyGive(led_controller_task_handle);
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}