#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

led_strip_handle_t strip = nullptr;
Color led_state[NUM_LEDS];

TaskHandle_t led_controller_task_handle = nullptr;

// Declaring a global variable
Params_t Params = {
    .brightness = 102,
    .color_index = 255,
    .mode = 0,
    .speed = 100,
};

// Creating a mutex
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

ParamsThread_t ParamsThread = {
    &Params,
    mutex
};


void led_controller_task(void *pvParameters)
{
    ParamsThread_t *thread_params = (ParamsThread_t *)pvParameters;

    while (1) {
        
        Params_t local_params;
        // Copy the settings (quick)
        xSemaphoreTake(thread_params->my_mutex, portMAX_DELAY);
        local_params = *thread_params->my_params;
        xSemaphoreGive(thread_params->my_mutex);

        // Select the animation based on the mode
        switch (local_params.mode) {
    	    case 0: anim_fill_step(local_params);               break;
    	    case 1: anim_scanner_step(local_params);            break;
    	    case 2: anim_blink_step(local_params);              break;
    	    case 3: anim_rainbow_step(local_params);            break;
            case 4: anim_halloween_step(local_params);          break;
    	    case 5: anim_halloween_pulse_step(local_params);    break;
    	    case 6: anim_ghost_step(local_params);              break;
            case 7: anim_random_points_step(local_params);      break;
            default:
                ESP_LOGE("ANIM_COUNT_ERROR", "Value %d, must be between %d and %d", local_params.mode, MIN_ANIM_COUNT, MAX_ANIM_COUNT);
                break;
	    }
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(local_params.speed));
    }
}


extern "C" void app_main()
{
    ESP_LOGI(TAG, "Demarrage...");

    // ---- GPIO button ----
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

    io_conf.pin_bit_mask =
        (1ULL << BRIGHTNESS_PIN) |
        (1ULL << COLOR_PIN) |
        (1ULL << MODE_PIN) |
        (1ULL << SPEED_PIN);
    gpio_config(&io_conf);

    // ---- LED strip ----
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_PIN,
        .max_leds = NUM_LEDS,
        .led_model = LED_MODEL_WS2812,
	.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color component format is G-R-B
    	.flags = {
        	.invert_out = false, // don't invert the output signal
    	}
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
    };

    led_strip_new_rmt_device(&strip_config, &rmt_config, &strip);

    // ---- TASKS ----
    xTaskCreatePinnedToCore(led_controller_task, "led_controller_task", LED_TASK_STACK_SIZE, &ParamsThread, 3, &led_controller_task_handle, LED_TASK_CORE);
    xTaskCreatePinnedToCore(brightness_listener, "brightness_listener", BUTTON_TASK_STACK_SIZE, &ParamsThread, 2, NULL, BUTTON_TASK_CORE);
    xTaskCreatePinnedToCore(color_listener, "color_listener", BUTTON_TASK_STACK_SIZE, &ParamsThread, 2, NULL, BUTTON_TASK_CORE);
    xTaskCreatePinnedToCore(speed_listener, "speed_listener", BUTTON_TASK_STACK_SIZE, &ParamsThread, 2, NULL, BUTTON_TASK_CORE);
    xTaskCreatePinnedToCore(animation_listener, "animation_listener", BUTTON_TASK_STACK_SIZE, &ParamsThread, 2, NULL, BUTTON_TASK_CORE);
}
