#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>
// --- Display Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define display_RESET -1
#define DISPLAY_REFRESH_MS 30 // Display refresh throttle (milliseconds) ~33 FPS

// TM1 temperature monitor conversion (TMP36-style default)
#define TEMP_SENSOR_VOLTAGE_OFFSET 0.5f
#define TEMP_SENSOR_V_PER_C 0.01f

// Battery monitor calibration factor.
// Two-point fit from measured values:
// 12.6V @ 2.62V ADC and 11.1V @ 2.40V ADC.
// This behaves like an ADC bias of ~0.715V before divider scaling.
#define BATTERY_ADC_OFFSET_V 0.715f

// --- Speed Configuration Structure ---
// These are runtime-mutable values, can be changed from menu
struct SpeedConfig
{
    int attack_speed;
    int search_speed;
    int turn_speed_aggressive;
    int turn_speed_moderate;
    int turn_speed_gentle;

    // Constructor with defaults
    SpeedConfig()
        : attack_speed(250),
          search_speed(128),
          turn_speed_aggressive(50),
          turn_speed_moderate(30),
          turn_speed_gentle(15) {}
};

// Global speed configuration (can be modified at runtime)
extern SpeedConfig speedConfig;

// IO Expander
#define IO_ADDRESS 0x22

enum ExpanderPin
{
    EXP_PIN_0 = 0,
    EXP_PIN_1 = 1,
    EXP_PIN_2 = 2,
    EXP_PIN_3 = 3,
    EXP_PIN_4 = 4,
    EXP_PIN_5 = 5,
    EXP_PIN_6 = 6,
    EXP_PIN_7 = 7
};

// 4-key keypad routing on MCP23X17 pins.
static const uint8_t KEYPAD_KEY_1_PIN = EXP_PIN_4;
static const uint8_t KEYPAD_KEY_2_PIN = EXP_PIN_5;
static const uint8_t KEYPAD_KEY_3_PIN = EXP_PIN_2;
static const uint8_t KEYPAD_KEY_4_PIN = EXP_PIN_3;

// Other MCP23X17 inputs on this PCB.
static const uint8_t INPUT_IR6_PIN = EXP_PIN_1;
static const uint8_t INPUT_CS_1_PIN = EXP_PIN_6;
static const uint8_t INPUT_CS_2_PIN = EXP_PIN_7;

// Vim-style keypad bindings (4 keys)
// h: left, j: down, k: up, l: right
static const uint8_t KEYPAD_VIM_H_PIN = KEYPAD_KEY_1_PIN;
static const uint8_t KEYPAD_VIM_J_PIN = KEYPAD_KEY_2_PIN;
static const uint8_t KEYPAD_VIM_K_PIN = KEYPAD_KEY_3_PIN;
static const uint8_t KEYPAD_VIM_L_PIN = KEYPAD_KEY_4_PIN;

#endif // DEFINES_H
