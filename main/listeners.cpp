#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include <utility>

#include "christmas_garland.h"


std::atomic<bool> auto_mode{false};

const std::pair<const char*, Color> colors[] =
{
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

    bool last_state = 1;
    int64_t press_start_time = 0;
    bool long_press_detected = false;

    while (1)
    {
        bool current_state = gpio_get_level(thread_params->button_PIN);

        // Button pressed
        if (current_state == 0 && last_state == 1)
        {
            press_start_time = esp_timer_get_time();
            long_press_detected = false;
        }

        // Button still pressed
        if (current_state == 0 && !long_press_detected)
        {
            int64_t elapsed_time =
                (esp_timer_get_time() - press_start_time) / 1000;

            if (elapsed_time >= LONG_PRESS_DURATION_MS)
            {
                long_press_detected = true;
            }
        }

        // Button released
        if (last_state == 0 && current_state == 1)
        {
            gpio_set_level(LED_BUILTIN, 1);
            // Lock the mutex
	        xSemaphoreTake( thread_params->mutex, portMAX_DELAY );

            if (!long_press_detected)
            {
                switch (thread_params->button_PIN)
                {
                case BRIGHTNESS_UP_PIN:
                {
                    thread_params->params->brightness = ( thread_params->params->brightness + MIN_BRIGHTNESS > MAX_BRIGHTNESS) ? MAX_BRIGHTNESS : thread_params->params->brightness + MIN_BRIGHTNESS;
                    ESP_LOGI("BTN", "Brightness Up: %d (max=255)", thread_params->params->brightness);
                    break;
                }

                case BRIGHTNESS_DOWN_PIN:
                {
                    thread_params->params->brightness = ( thread_params->params->brightness - MIN_BRIGHTNESS < MIN_BRIGHTNESS) ? MIN_BRIGHTNESS : thread_params->params->brightness - MIN_BRIGHTNESS;
                    ESP_LOGI("BTN", "Brightness Down: %d (min=51)", thread_params->params->brightness);
                    break;
                }

                case COLOR_PIN:
                {
                    thread_params->params->color_index = (thread_params->params->color_index + 1) % NUM_COLORS;
                    ESP_LOGI("BTN", "Color: %s (index=%d)", colors[thread_params->params->color_index].first, thread_params->params->color_index);
                    break;
                }

                case MODE_PIN:
                {
                    if (auto_mode.load())
                    {
                        ESP_LOGW("BTN", "Stop automatic mode"); 
                    }
                    auto_mode.store(false);
                    thread_params->params->mode = (thread_params->params->mode + 1 >= themes[thread_params->params->theme].mode_count) ? 0 :  thread_params->params->mode + 1;


                    break;
                }

                case SPEED_UP_PIN:
                {
                    thread_params->params->speed = (thread_params->params->speed - MIN_SPEED_MS < MIN_SPEED_MS) ? MIN_SPEED_MS :  thread_params->params->speed - MIN_SPEED_MS;
                    ESP_LOGI("BTN", "Speed Up: %dms (min=100ms)", thread_params->params->speed);
                    break;
                }
                
                case SPEED_DOWN_PIN:
                {
                    thread_params->params->speed = (thread_params->params->speed + MIN_SPEED_MS > MAX_SPEED_MS) ? MAX_SPEED_MS :  thread_params->params->speed + MIN_SPEED_MS;
                    ESP_LOGI("BTN", "Speed Down: %dms (max=500ms)", thread_params->params->speed);
                    break;
                }

                default:
                    break;
                }
            }
            else
            {
                switch (thread_params->button_PIN)
                { 
                case COLOR_PIN:
                {
                    thread_params->params->theme = (thread_params->params->theme + 1 > NUM_THEMES) ? 0 :  thread_params->params->theme + 1;
                    ESP_LOGI("BTN", "Switching to theme: %s", themes[thread_params->params->theme].name);
                    break;
                }

                case MODE_PIN:
                {
                    auto_mode.store(true);
                    ESP_LOGW("BTN", "Start automatic mode"); 
                    break;
                }

                default:
                    break;
                }
            }
            // Unlock the mutex
            xSemaphoreGive( thread_params->mutex );
            xTaskNotifyGive(led_controller_handle);
            gpio_set_level(LED_BUILTIN, 0);
        }
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}