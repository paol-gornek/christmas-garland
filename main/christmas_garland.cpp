#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"

#define TAG "APP"

// ===== CONFIG =====
#define LED_PIN     13
#define NUM_LEDS    60

#define BRIGHTNESS_PIN	GPIO_NUM_14
#define COLOR_PIN	GPIO_NUM_27
#define MODE_PIN	GPIO_NUM_26
#define SPEED_PIN	GPIO_NUM_25

// ===== GLOBAL =====
led_strip_handle_t strip;
//uint8_t color_index = 255; // couleur actuelle
//uint8_t brightness = 102;
//bool led_update_needed = true;

struct Color { uint8_t r, g, b; };
Color led_state[NUM_LEDS]; // couleur actuelle de chaque LED

struct Params_t {
    uint16_t brightness;
    uint8_t color_index;
    uint8_t mode;
    uint16_t speed;
    bool led_update_needed;
};

// Declaring a global variable
Params_t Params = {
    .brightness = 102,
    .color_index = 255,
    .mode = 1,
    .speed = 100,
    .led_update_needed = true
};

// Creating a mutex
SemaphoreHandle_t mutex;


// Color tab
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

void anim_fill_step(Params_t p)
{
    static int i = 0;

    Color base = colors[p.color_index];
    uint8_t r = (base.r * p.brightness) / 255;
    uint8_t g = (base.g * p.brightness) / 255;
    uint8_t b = (base.b * p.brightness) / 255;

    led_strip_set_pixel(strip, i, r, g, b);
    led_strip_refresh(strip);

    i++;
    if (i >= NUM_LEDS)
        i = 0;
}


void anim_scanner_step(Params_t p)
{
    static int pos = 0;
    static int dir = 1;

    led_strip_clear(strip);

    Color base = colors[p.color_index];
    uint8_t r = (base.r * p.brightness) / 255;
    uint8_t g = (base.g * p.brightness) / 255;
    uint8_t b = (base.b * p.brightness) / 255;

    led_strip_set_pixel(strip, pos, r, g, b);
    led_strip_refresh(strip);

    pos += dir;
    if (pos == NUM_LEDS - 1 || pos == 0)
        dir = -dir;
}


void anim_blink_step(Params_t p)
{
    static bool on = false;

    Color base = colors[p.color_index];
    uint8_t r = on ? (base.r * p.brightness) / 255 : 0;
    uint8_t g = on ? (base.g * p.brightness) / 255 : 0;
    uint8_t b = on ? (base.b * p.brightness) / 255 : 0;

    for (int i = 0; i < NUM_LEDS; i++)
        led_strip_set_pixel(strip, i, r, g, b);

    led_strip_refresh(strip);
    on = !on;
}


static Color wheel(uint8_t pos)
{
    if (pos < 85) {
        return {
            (uint8_t)(pos * 3),
            (uint8_t)(255 - pos * 3),
            0
        };
    }

    if (pos < 170) {
        pos -= 85;
        return {
            (uint8_t)(255 - pos * 3),
            0,
            (uint8_t)(pos * 3)
        };
    }

    pos -= 170;
    return {
        0,
        (uint8_t)(pos * 3),
        (uint8_t)(255 - pos * 3)
    };
}


void anim_rainbow_step(Params_t p)
{
    static uint8_t offset = 0;

    for (int i = 0; i < NUM_LEDS; i++) {
        Color c = wheel(i * 256 / NUM_LEDS + offset);
        c.r = (c.r * p.brightness) / 255;
        c.g = (c.g * p.brightness) / 255;
        c.b = (c.b * p.brightness) / 255;

        led_strip_set_pixel(strip, i, c.r, c.g, c.b);
    }

    led_strip_refresh(strip);
    offset++;
}



void led_task(void *arg)
{
    Params_t local_params;

    while (1) {

        // Copy the settings (quick)
        xSemaphoreTake(mutex, portMAX_DELAY);
        local_params = Params;
        xSemaphoreGive(mutex);

        // Select the animation based on the mode
        switch (local_params.mode) {
    	    case 0: anim_fill_step(local_params);    break;
    	    case 1: anim_scanner_step(local_params); break;
    	    case 2: anim_blink_step(local_params);   break;
    	    case 3: anim_rainbow_step(local_params); break;
	}

        vTaskDelay(pdMS_TO_TICKS(local_params.speed));
    }
}




void brightness_task(void *arg)
{
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
	    xSemaphoreTake( mutex, portMAX_DELAY );

            if (duration > pdMS_TO_TICKS(700)) {
                 Params.brightness = ( Params.brightness <= 51) ? 255 :  Params.brightness - 51;
                ESP_LOGI("BTN", "Long press → brightness=%d", Params.brightness);
            } else {
                 Params.brightness = ( Params.brightness + 51 > 255) ? 51 :  Params.brightness + 51;
                ESP_LOGI("BTN", "Short press → brightness=%d", Params.brightness);
            }

            Params.led_update_needed = true;
	    
	    // Unlock the mutex
	    xSemaphoreGive( mutex );


        }

        last = now;
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

void color_task(void *arg)
{
    bool last = 1;

    while (1) {
        bool now = gpio_get_level(COLOR_PIN);

	// Lock the mutex
	xSemaphoreTake( mutex, portMAX_DELAY );

        if (now == 0 && last == 1) {
            Params.color_index = (Params.color_index + 1) % NUM_COLORS;
            ESP_LOGI("COLOR", "Color index=%d", Params.color_index);
            Params.led_update_needed = true;
        }

	// Unlock the mutex
	xSemaphoreGive( mutex );

        last = now;
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void mode_task(void *arg)
{
    bool last = 1;

    while (1) {
        bool now = gpio_get_level(MODE_PIN);

	// Lock the mutex
	xSemaphoreTake( mutex, portMAX_DELAY );

        if (now == 0 && last == 1) {
            Params.mode = (Params.mode + 1 > 8) ? 1 :  Params.mode + 1;
            ESP_LOGI("COLOR", "Mode=%d", Params.mode);
            Params.led_update_needed = true;
        }

	// Unlock the mutex
	xSemaphoreGive( mutex );

        last = now;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void speed_task(void *arg)
{
    bool last = 1;

    while (1) {
        bool now = gpio_get_level(SPEED_PIN);

	// Lock the mutex
	xSemaphoreTake( mutex, portMAX_DELAY );

        if (now == 0 && last == 1) {
            Params.speed = (Params.speed + 100 > 500) ? 100 :  Params.speed + 100;
            ESP_LOGI("COLOR", "Speed=%d", Params.speed);
            Params.led_update_needed = true;
        }

	// Unlock the mutex
	xSemaphoreGive( mutex );

        last = now;
        vTaskDelay(pdMS_TO_TICKS(200));
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

    mutex = xSemaphoreCreateMutex();


    // ---- TASKS ----
    xTaskCreatePinnedToCore(led_task, "led_task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(brightness_task, "brightness_task", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(color_task, "color_task", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(speed_task, "speed_task", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(mode_task, "mode_task", 2048, NULL, 3, NULL, 1);
}
