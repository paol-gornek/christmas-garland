#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

void brightness_listener(void *pvParameters)
{
    ParamsThread_t *thread_params = (ParamsThread_t *)pvParameters;

    bool last = 1;
    TickType_t press_time = 0;

    while (1) {
        bool now = gpio_get_level(BRIGHTNESS_PIN);

        if (now == 0 && last == 1) {
            press_time = xTaskGetTickCount();
        }

        if (now == 1 && last == 0) {
            TickType_t duration = xTaskGetTickCount() - press_time;

            // Lock the mutex
            xSemaphoreTake( thread_params->my_mutex, portMAX_DELAY );

            if (duration > pdMS_TO_TICKS(700)) {
                thread_params->my_params->brightness = ( thread_params->my_params->brightness <= 51) ? 255 : thread_params->my_params->brightness - 51;
                ESP_LOGI("BTN", "Long press → brightness=%d", thread_params->my_params->brightness);
            } else {
                thread_params->my_params->brightness = ( thread_params->my_params->brightness + 51 > 255) ? 51 : thread_params->my_params->brightness + 51;
                ESP_LOGI("BTN", "Short press → brightness=%d", thread_params->my_params->brightness);
            }
	    
            // Unlock the mutex
            xSemaphoreGive( thread_params->my_mutex );
            xTaskNotifyGive(led_controller_task_handle);
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}