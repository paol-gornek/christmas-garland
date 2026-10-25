#ifndef CHRISTMAS_GARLAND_H
#define CHRISTMAS_GARLAND_H

#include <stdint.h>
#include <string>

#define CFG "CONFIG"
#define APP "APPLI"


// LED strip configuration and pin definitions
constexpr gpio_num_t LED_STRIP_DATA_PIN = GPIO_NUM_13;
constexpr uint16_t NUM_LEDS = 120;
constexpr gpio_num_t BRIGHTNESS_PIN = GPIO_NUM_14;
constexpr gpio_num_t COLOR_PIN = GPIO_NUM_27;
constexpr gpio_num_t MODE_PIN = GPIO_NUM_26;
constexpr gpio_num_t SPEED_PIN = GPIO_NUM_25;

// Task stack sizes (bytes)
constexpr uint32_t LED_TASK_STACK_SIZE = 4096;
constexpr uint32_t BUTTON_TASK_STACK_SIZE = 2048;

// ESP32 core number
constexpr int LED_TASK_CORE = 0;
constexpr int BUTTON_TASK_CORE = 1;

// Allowed range (min/max)
constexpr int MIN_BRIGHTNESS = 51;
constexpr int MAX_BRIGHTNESS = 255;
constexpr int MIN_MODE_INDEX = 0;
constexpr int MAX_MODE_INDEX = 7;
constexpr int MIN_SPEED_MS = 100;
constexpr int MAX_SPEED_MS = 500;

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

//  task function
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

#endif