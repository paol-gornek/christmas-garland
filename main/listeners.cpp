#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

#include <utility>

const std::pair<const char*, Color> colors[] = {
    {"red",     {255, 0, 0}},
    {"green",   {0, 255, 0}},
    {"blue",    {0, 0, 255}},
    {"yellow",  {255, 255, 0}},
    {"cyan",    {0, 255, 255}},
    {"magenta", {255, 0, 255}},
    {"orange",  {255, 128, 0}},
    {"purple",  {128, 0, 255}},
    {"pink",    {255, 20, 147}},
    {"white",   {255, 255, 255}},
    {"brown",   {139, 69, 19}},
    {"gold",    {255, 215, 0}},
    {"teal",    {0, 128, 128}},
    {"lime",    {128, 255, 0}},
    {"coral",   {255, 127, 80}}
};
const std::size_t NUM_COLORS = std::size(colors);

void listener(void *pvParameters)
{
    ListenerTask_t *thread_params = (ListenerTask_t *)pvParameters;

    bool last = 1;

    while (1) {
        bool now = gpio_get_level(thread_params->button_PIN);

        if (now == 0 && last == 1) {
            // Lock the mutex
	        xSemaphoreTake( thread_params->mutex, portMAX_DELAY );

            switch (thread_params->button_PIN)
            {
            case BRIGHTNESS_PIN:
                thread_params->params->brightness = ( thread_params->params->brightness + MIN_BRIGHTNESS > MAX_BRIGHTNESS) ? MIN_BRIGHTNESS : thread_params->params->brightness + MIN_BRIGHTNESS;
                ESP_LOGI("BTN", "Brightness: %d (max=255)", thread_params->params->brightness);
                break;
            
            case COLOR_PIN:
                thread_params->params->color_index = (thread_params->params->color_index + 1) % NUM_COLORS;
                ESP_LOGI("BTN", "Color: %s (index=%d)", colors[thread_params->params->color_index].first, thread_params->params->color_index);
                break;

            case MODE_PIN:
                thread_params->params->mode = (thread_params->params->mode + 1 > MAX_MODE_INDEX) ? MIN_MODE_INDEX :  thread_params->params->mode + 1;
                ESP_LOGI("BTN", "Mode index=%d", thread_params->params->mode);
                break;

            case SPEED_PIN:
                thread_params->params->speed = (thread_params->params->speed + MIN_SPEED_MS > MAX_SPEED_MS) ? MIN_SPEED_MS :  thread_params->params->speed + MIN_SPEED_MS;
                ESP_LOGI("BTN", "Speed: %dms (max=500ms)", thread_params->params->speed);
                break;

            default:
                break;
            }

            // Unlock the mutex
	        xSemaphoreGive( thread_params->mutex );
            xTaskNotifyGive(led_controller_handle);
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}