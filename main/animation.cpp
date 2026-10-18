#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "christmas_garland.h"

#include "esp_random.h"
#include <cmath>

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

void animation_mode_task(void *pvParameters)
{
    ParamsThread_t *thread_params = (ParamsThread_t *)pvParameters;
    bool last = 1;

    while (1) {
        bool now = gpio_get_level(MODE_PIN);

	// Lock the mutex
	xSemaphoreTake( thread_params->my_mutex, portMAX_DELAY );

        if (now == 0 && last == 1) {
            thread_params->my_params->mode = (thread_params->my_params->mode + 1 > 8) ? 1 :  thread_params->my_params->mode + 1;
            ESP_LOGI("MODE", "Mode=%d", thread_params->my_params->mode);
            thread_params->my_params->led_update_needed = true;
        }

	// Unlock the mutex
	xSemaphoreGive( thread_params->my_mutex );

        last = now;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

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

void anim_halloween_step(Params_t p)
{
    static uint8_t flicker[NUM_LEDS] = {0};

    for (int i = 0; i < NUM_LEDS; i++) {
        uint32_t rnd = esp_random();

        if (flicker[i] == 0) {
            if ((rnd % 10) == 0) {
                flicker[i] = 100 + (rnd % 156);
            }
        } else {
            if ((rnd % 8) == 0) {
                flicker[i] = 0;
            } else if ((rnd % 3) == 0) {
                flicker[i] = 100 + (rnd % 156);
            }
        }

        uint8_t r = (255 * p.brightness / 255) * flicker[i] / 255;
        uint8_t g = ( 80 * p.brightness / 255) * flicker[i] / 255;
        uint8_t b = 0;

        led_strip_set_pixel(strip, i, r, g, b);
    }

    led_strip_refresh(strip);
}

void anim_halloween_pulse_step(Params_t p)
{
    static int frame = 0;

    uint8_t pulse = (uint8_t)(128 + 127 * sinf(frame * 0.12f));

    for (int i = 0; i < NUM_LEDS; i++) {

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        int distance = abs(i - NUM_LEDS / 2);

        if (distance < 12) {
            r = (255 * pulse / 255) * p.brightness / 255;
            g = (70  * pulse / 255) * p.brightness / 255;
            b = 0;
        }

        else if (esp_random() % 25 == 0) {
            r = 255 * p.brightness / 255;
            g = 100 * p.brightness / 255;
            b = 0;
        }

        led_strip_set_pixel(strip, i, r, g, b);
    }

    led_strip_refresh(strip);

    frame++;
}

void anim_ghost_step(Params_t p)
{
    static int pos = 0;
    static int dir = 1;

    for (int i = 0; i < NUM_LEDS; i++) {
        int d = abs(i - pos);

        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        if (d == 0) {
            r = 220;
            g = 220;
            b = 255;
        }
        else if (d <= 3) {
            uint8_t intensity = 180 / d;
            r = intensity / 2;
            g = 0;
            b = intensity;
        }
        else if (d <= 8) {
            uint8_t intensity = 50 / (d - 2);
            r = intensity / 3;
            g = 0;
            b = intensity;
        }

        r = (uint16_t)r * p.brightness / 255;
        g = (uint16_t)g * p.brightness / 255;
        b = (uint16_t)b * p.brightness / 255;

        led_strip_set_pixel(strip, i, r, g, b);
    }

    led_strip_refresh(strip);

    pos += dir;

    if (pos >= NUM_LEDS - 1) {
        pos = NUM_LEDS - 1;
        dir = -1;
    }
    else if (pos <= 0) {
        pos = 0;
        dir = 1;
    }
}

void anim_random_points_step(Params_t p)
{
    static int pos[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    static int dir[8] = {1, -1, 1, -1, 1, -1, 1, -1};
    static uint8_t life[8] = {0};

    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(strip, i, 0, 0, 0);
    }

    for (int j = 0; j < 8; j++) {

        if (life[j] == 0) {
            if (esp_random() % 4 == 0) {
                pos[j] = esp_random() % NUM_LEDS;
                dir[j] = (esp_random() % 2) ? 1 : -1;
                life[j] = 10 + esp_random() % 20;
            }
            continue;
        }

        uint8_t r = 255;
        uint8_t g = 0;
        uint8_t b = 0;

        switch (esp_random() % 3) {
            case 0:
                r = 255; g = 70; b = 0;
                break;

            case 1:
                r = 100; g = 0; b = 255;
                break;

            case 2:
                r = 180; g = 180; b = 255;
                break;
        }

        r = (uint16_t)r * p.brightness / 255;
        g = (uint16_t)g * p.brightness / 255;
        b = (uint16_t)b * p.brightness / 255;

        led_strip_set_pixel(strip, pos[j], r, g, b);

        if (esp_random() % 3 != 0) {
            pos[j] += dir[j];
        }

        if (esp_random() % 5 == 0) {
            dir[j] = -dir[j];
        }

        if (pos[j] >= NUM_LEDS - 1) {
            pos[j] = NUM_LEDS - 1;
            dir[j] = -1;
        }
        else if (pos[j] <= 0) {
            pos[j] = 0;
            dir[j] = 1;
        }

        life[j]--;
    }

    led_strip_refresh(strip);
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
