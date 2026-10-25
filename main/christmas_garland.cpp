#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

led_strip_handle_t strip_handle = nullptr;
TaskHandle_t led_controller_handle = nullptr;

// Declaring a default variable
LedParameters_t shared_params = {
    .brightness = 102,
    .color_index = 1,
    .mode = 0,
    .speed = 100,
};

// Creating a mutex
SemaphoreHandle_t params_mutex = xSemaphoreCreateMutex();


ControllerTask_t ControllerThread = {
    &shared_params,
    params_mutex,
};

ListenerTask_t brightness_config = {&shared_params, params_mutex, BRIGHTNESS_PIN};
ListenerTask_t color_config = {&shared_params, params_mutex, COLOR_PIN};
ListenerTask_t mode_config = {&shared_params, params_mutex, MODE_PIN};
ListenerTask_t speed_config = {&shared_params, params_mutex, SPEED_PIN};


void led_controller_task(void *pvParameters)
{
    ControllerTask_t *thread_params = (ControllerTask_t *)pvParameters;

    while (1) {
        
        LedParameters_t local_params;
        // Copy the settings (quick)
        xSemaphoreTake(thread_params->mutex, portMAX_DELAY);
        local_params = *thread_params->params;
        xSemaphoreGive(thread_params->mutex);

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
                ESP_LOGE("MODE_INDEX_ERROR", "Value %d, must be between %d and %d", local_params.mode, MIN_MODE_INDEX, MAX_MODE_INDEX);
                break;
	    }
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(local_params.speed));
    }
}


extern "C" void app_main()
{
    ESP_LOGI(APP, "Starting Christmas garland...");

    // ---- GPIO common configurations ----
    gpio_config_t io_conf = {
        .pin_bit_mask =
            (1ULL << BRIGHTNESS_PIN) |
            (1ULL << COLOR_PIN) |
            (1ULL << MODE_PIN) |
            (1ULL << SPEED_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t gpio_err = gpio_config(&io_conf);
    if (gpio_err != ESP_OK) {
        ESP_LOGE(CFG, "Failed to configure GPIOs: %s",
                esp_err_to_name(gpio_err));
        return;
    }
    ESP_LOGI(CFG, "GPIOs configured successfully");


    // ---- LED Strip common configurations ----
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_DATA_PIN,
        .max_leds = NUM_LEDS,
        .led_model = LED_MODEL_WS2812,
	    .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // Green Red Blue
    	.flags = {
        	.invert_out = false,
    	}
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .mem_block_symbols = 0,
        .flags = {
            .with_dma = false,  // DMA disabled
        }
    };

    // Initializing LED strip control device
    esp_err_t device_err = led_strip_new_rmt_device(&strip_config, &rmt_config, &strip_handle);
    if (device_err != ESP_OK) {
        ESP_LOGE(CFG, "Failed to create LED strip device: %s",
                esp_err_to_name(device_err));
        return;
    }
    ESP_LOGI(CFG, "LED strip device created successfully");



    // ---- Configuration summary ----
    ESP_LOGI(CFG, "========================================");
    ESP_LOGI(CFG, "       CONFIGURATION SUMMARY");
    ESP_LOGI(CFG, "========================================");

    ESP_LOGI(CFG, "LED strip:");
    ESP_LOGI(CFG, "  Model          : WS2812");
    ESP_LOGI(CFG, "  Number of LEDs : %u", (unsigned)NUM_LEDS);
    ESP_LOGI(CFG, "  Data GPIO      : %d", (int)LED_STRIP_DATA_PIN);
    ESP_LOGI(CFG, "  Color order    : GRB");

    ESP_LOGI(CFG, "Button GPIOs:");
    ESP_LOGI(CFG, "  Brightness     : GPIO %d", (int)BRIGHTNESS_PIN);
    ESP_LOGI(CFG, "  Color          : GPIO %d", (int)COLOR_PIN);
    ESP_LOGI(CFG, "  Mode           : GPIO %d", (int)MODE_PIN);
    ESP_LOGI(CFG, "  Speed          : GPIO %d", (int)SPEED_PIN);

    ESP_LOGI(CFG, "GPIO settings:");
    ESP_LOGI(CFG, "  Mode           : INPUT");
    ESP_LOGI(CFG, "  Pull-up        : ENABLED");
    ESP_LOGI(CFG, "  Polling period : 50 ms");

    ESP_LOGI(CFG, "RMT settings:");
    ESP_LOGI(CFG, "  Clock source   : DEFAULT");
    ESP_LOGI(CFG, "  Resolution     : %u MHz",
            (unsigned)(rmt_config.resolution_hz / 1000000U));

    ESP_LOGI(CFG, "========================================");


    // ---- FreeRTOS task creation and error handling ----
    BaseType_t tab[] = {
        xTaskCreatePinnedToCore(led_controller_task, "led_controller_task", LED_TASK_STACK_SIZE, &ControllerThread, 3, &led_controller_handle, LED_TASK_CORE),
        xTaskCreatePinnedToCore(listener, "brightness_listener", BUTTON_TASK_STACK_SIZE, &brightness_config, 2, NULL, BUTTON_TASK_CORE),
        xTaskCreatePinnedToCore(listener, "color_listener", BUTTON_TASK_STACK_SIZE, &color_config, 2, NULL, BUTTON_TASK_CORE),
        xTaskCreatePinnedToCore(listener, "mode_listener", BUTTON_TASK_STACK_SIZE, &mode_config, 2, NULL, BUTTON_TASK_CORE),
        xTaskCreatePinnedToCore(listener, "speed_listener", BUTTON_TASK_STACK_SIZE, &speed_config, 2, NULL, BUTTON_TASK_CORE),
    };

    std::string result = "";

    for (int i = 0; i < 5; i++) {
        if (tab[i] != pdPASS) {
            switch (i) {
                case 0: result += "led_controller_task, "; break;
                case 1: result += "brightness_listener, "; break;
                case 2: result += "color_listener, "; break;
                case 3: result += "mode_listener, "; break;
                case 4: result += "speed_listener, "; break;
            }
        }
    }

    if (!result.empty()) {
        ESP_LOGE(APP, "Failed to create task(s): %s", result.c_str());
        return;
    }
    ESP_LOGI(APP, "All tasks were successfully created");
}
