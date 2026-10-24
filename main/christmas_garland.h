#ifndef CHRISTMAS_GARLAND_H
#define CHRISTMAS_GARLAND_H

#define TAG "APP"

// ===== CONFIG =====
#define LED_PIN     13
#define NUM_LEDS    60

#define BRIGHTNESS_PIN  GPIO_NUM_14
#define COLOR_PIN       GPIO_NUM_27
#define MODE_PIN        GPIO_NUM_26
#define SPEED_PIN       GPIO_NUM_25


// COLOR
// Color tab
struct Color { uint8_t r, g, b; };

// ===== GLOBAL =====
extern led_strip_handle_t strip;
extern const Color colors[];
extern const std::size_t NUM_COLORS;

struct Params_t {
    uint16_t brightness;
    uint8_t color_index;
    uint8_t mode;
    uint16_t speed;
    bool led_update_needed;
};

struct ParamsThread_t {
    Params_t *my_params;
    SemaphoreHandle_t my_mutex;
};


void speed_listener(void *arg);
void color_listener(void *arg);
void brightness_listener(void *arg);
void animation_listener(void *arg);


void anim_fill_step(Params_t p);
void anim_scanner_step(Params_t p);
void anim_blink_step(Params_t p);
void anim_rainbow_step(Params_t p);
void anim_halloween_step(Params_t p);
void anim_halloween_pulse_step(Params_t p);
void anim_ghost_step(Params_t p);
void anim_random_points_step(Params_t p);

#endif