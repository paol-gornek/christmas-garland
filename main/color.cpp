#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

const Color colors[] = {
    {255, 0, 0},     // red
    {0, 255, 0},     // green
    {0, 0, 255},     // blue
    {255, 255, 0},   // yellow
    {0, 255, 255},   // cyan
    {255, 0, 255},   // magenta
    {255, 128, 0},   // orange
    {128, 0, 255},   // purp
    {255, 192, 203}, // pink
    {255, 255, 255}  // white
};
#define NUM_COLORS (sizeof(colors)/sizeof(colors[0]))

void color_task(void *pvParameters)
{
    ParamsThread_t *thread_params = (ParamsThread_t *)pvParameters;

    bool last = 1;

    while (1) {
        bool now = gpio_get_level(COLOR_PIN);

	// Lock the mutex
	xSemaphoreTake( thread_params->my_mutex, portMAX_DELAY );

        if (now == 0 && last == 1) {
            thread_params->my_params->color_index = (thread_params->my_params->color_index + 1) % NUM_COLORS;
            ESP_LOGI("COLOR", "Color index=%d", thread_params->my_params->color_index);
            thread_params->my_params->led_update_needed = true;
        }

	// Unlock the mutex
	xSemaphoreGive( thread_params->my_mutex );

        last = now;
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}