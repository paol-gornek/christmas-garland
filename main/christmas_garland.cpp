#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

led_strip_handle_t strip = nullptr;
Color led_state[NUM_LEDS]; // couleur actuelle de chaque LED

// Declaring a global variable
Params_t Params = {
    .brightness = 102,
    .color_index = 255,
    .mode = 1,
    .speed = 100,
    .led_update_needed = true
};

// Creating a mutex
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

ParamsThread_t ParamsThread = {
    &Params,
    mutex
};


void led_task(void *pvParameters)
{
    ParamsThread_t *thread_params = (ParamsThread_t *)pvParameters;
    Params_t local_params;

    while (1) {

        // Copy the settings (quick)
        xSemaphoreTake(thread_params->my_mutex, portMAX_DELAY);
        local_params = *thread_params->my_params;
        xSemaphoreGive(thread_params->my_mutex);

        // Select the animation based on the mode
        switch (local_params.mode) {
    	    case 0: anim_fill_step(local_params);    break;
    	    case 1: anim_scanner_step(local_params); break;
    	    case 2: anim_blink_step(local_params);   break;
    	    case 3: anim_rainbow_step(local_params); break;
            case 4: anim_halloween_step(local_params); break;
    	    case 5: anim_halloween_pulse_step(local_params); break;
    	    case 6: anim_ghost_step(local_params); break;
	}

        vTaskDelay(pdMS_TO_TICKS(local_params.speed));
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
    xTaskCreatePinnedToCore(led_task, "led_task", 4096, &ParamsThread, 5, NULL, 0);
    xTaskCreatePinnedToCore(brightness_task, "brightness_task", 2048, &ParamsThread, 3, NULL, 1);
    xTaskCreatePinnedToCore(color_task, "color_task", 2048, &ParamsThread, 3, NULL, 1);
    xTaskCreatePinnedToCore(speed_task, "speed_task", 2048, &ParamsThread, 3, NULL, 1);
    xTaskCreatePinnedToCore(animation_mode_task, "animation_mode_task", 2048, &ParamsThread, 3, NULL, 1);
}
