#ifndef CHRISTMAS_GARLAND_H
#define CHRISTMAS_GARLAND_H

#include <atomic>
#include <stdint.h>
#include <string>

#define CFG "CONFIG"
#define APP "APPLI"

// ---- On-board LED configurations ----
constexpr gpio_num_t LED_BUILTIN = GPIO_NUM_2;


// LED strip configuration and pin definitions
constexpr gpio_num_t LED_STRIP_DATA_PIN = GPIO_NUM_13;
constexpr uint16_t NUM_LEDS = 120;
constexpr gpio_num_t BRIGHTNESS_UP_PIN = GPIO_NUM_32;
constexpr gpio_num_t BRIGHTNESS_DOWN_PIN = GPIO_NUM_33;
constexpr gpio_num_t COLOR_PIN = GPIO_NUM_25;
constexpr gpio_num_t MODE_PIN = GPIO_NUM_26;
constexpr gpio_num_t SPEED_UP_PIN = GPIO_NUM_27;
constexpr gpio_num_t SPEED_DOWN_PIN = GPIO_NUM_14;

// Task stack sizes (bytes)
constexpr uint32_t LED_TASK_STACK_SIZE = 4096;
constexpr uint32_t BUTTON_TASK_STACK_SIZE = 2048;

// ESP32 core number
constexpr int LED_TASK_CORE = 0;
constexpr int BUTTON_TASK_CORE = 1;

// Allowed range (min/max)
constexpr int MIN_BRIGHTNESS = 51;
constexpr int MAX_BRIGHTNESS = 255;
constexpr int NUM_MODE = 15;
constexpr int MIN_SPEED_MS = 100;
constexpr int MAX_SPEED_MS = 500;

extern int NUM_THEMES;

// Global handles
extern led_strip_handle_t strip_handle;
extern TaskHandle_t led_controller_handle;

// Color definitions
struct Color { uint8_t r, g, b; };
extern const std::pair<const char*, Color> colors[];
extern const std::size_t NUM_COLORS;

// LED parameters
struct LedParameters_t {
    uint16_t brightness;
    uint8_t color_index;
    uint8_t mode;
    uint8_t theme;
    uint16_t speed;
};

// Controller task context
struct ControllerTask_t {
    LedParameters_t *params;
    SemaphoreHandle_t mutex;
};

// Listener task context
struct ListenerTask_t {
    LedParameters_t *params;
    SemaphoreHandle_t mutex;
    gpio_num_t button_PIN;
};

// theme struct
struct Theme {
    const char* name;
    int modes[NUM_MODE];
    int mode_count;
};
extern Theme themes[];

//  task function
constexpr int LONG_PRESS_DURATION_MS = 1000;
constexpr int AUTO_MODE_DURATION_MS = 180000;
extern std::atomic<bool> auto_mode;
void listener(void *arg);

// LED animation functions
void anim_fill_step(LedParameters_t params);
void anim_scanner_step(LedParameters_t params);
void anim_blink_step(LedParameters_t params);
void anim_rainbow_step(LedParameters_t params);
void anim_halloween_step(LedParameters_t params);
void anim_halloween_pulse_step(LedParameters_t params);
void anim_ghost_step(LedParameters_t params);
void anim_random_points_step(LedParameters_t params);
void anim_breathing_step(LedParameters_t params);
void anim_stars_step(LedParameters_t params);
void anim_christmas_lights_step(LedParameters_t params);
void anim_candy_cane_step(LedParameters_t params);
void anim_snowfall_step(LedParameters_t params);
void anim_golden_comet_step(LedParameters_t params);
void anim_christmas_sparkles_step(LedParameters_t params);

#endif