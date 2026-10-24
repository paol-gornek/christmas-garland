# Christmas Garland 🎄

A Christmas LED garland project based on an ESP32, developed in C++ using ESP-IDF.

## 🎯 Project Goal

The goal of this project is to create a programmable Christmas LED garland with custom lighting effects.

## 🛠️ Development

This project uses [ESP-IDF](https://github.com/espressif/esp-idf), Espressif's official development framework for ESP32 devices.

### Prerequisites

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) installed and configured
- Visual Studio Code (optional)
- An ESP32 development board

## 🔌 Hardware

- ESP32 development board
- Addressable LED strip with 60 LEDs (WS2812B)
- 3.7 V, 400 mAh battery
- Slide switch for power control

The electronic schematic is available here:

![View the schematic](/docs/wiring_diagram.png)

### ESP32 Pin Configuration

| ESP32 Pin | Function    | Description                              |
|-----------|-------------|------------------------------------------|
| GPIO 13   | LED Data    | Sends control data to the LED strip      |
| GPIO 14   | Brightness  | Adjusts the brightness of the LEDs       |
| GPIO 27   | Color       | Selects the LED color                    |
| GPIO 26   | Mode        | Selects the lighting pattern (e.g.       |
|           |             | snake, rainbow, etc.)                    |
| GPIO 25   | Speed       | Adjusts the speed of the lighting effect |

### Button Controls

Each of the four control buttons runs in its own FreeRTOS task (thread), allowing the controls to operate independently.

- **Brightness Button:** Adjusts LED brightness.
- **Color Button:** Changes the LED color.
- **Mode Button:** Selects the lighting pattern (e.g., snake, rainbow, etc.).
- **Speed Button:** Adjusts the speed of the lighting effects.

## Build

Activate the ESP-IDF environment:
*   ```bash
    source "$HOME/.espressif/v6.1/esp-idf/export.sh"
    ```

Build / Flash the project:
*   ```bash
    idf.py build
    ```
*   ```bash
    idf.py -p /dev/ttyXXX flash
    ```

Display serial output:
*   ```bash
    idf.py monitor
    ```
    *Ctrl + ] - to exit the monitor*

Clean the project, delete build output files or entire build directory:
*   ```bash
    idf.py -p /dev/ttyXXX flash
    ```
*   ```bash
    idf.py fullclean
    ```