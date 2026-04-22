#ifndef MENU_H
#define MENU_H

// Menu screen indices
#define MENU_SCREEN_MAIN 0
#define MENU_SCREEN_SPEED 1
#define MENU_SCREEN_IR 2
#define MENU_SCREEN_STRATEGY 3
#define MENU_SCREEN_DIRECTION 4
#define MENU_SCREEN_BATTERY 5
#define MENU_SCREEN_TEMP 6
#define MENU_SCREEN_COUNT 7

// Robot mode enumeration
enum RobotMode
{
    MODE_MENU,    // In menu navigation mode
    MODE_RUNNING, // Running normally (IR behavior active)
    MODE_PAUSED   // Paused (sensors active, motors stopped)
};

// Speed level enumeration
enum SpeedLevel
{
    SPEED_LEVEL_LOW = 0,
    SPEED_LEVEL_MEDIUM = 1,
    SPEED_LEVEL_HIGH = 2,
    SPEED_LEVEL_COUNT = 3
};

// Speed preset values (for each level)
struct SpeedPreset
{
    uint16_t attack;
    uint16_t search;
    uint16_t turn_moderate;
    uint16_t turn_gentle;
};

// Preset configurations
static const SpeedPreset SPEED_PRESETS[SPEED_LEVEL_COUNT] = {
    // LOW
    {60, 40, 40, 10},
    // MEDIUM
    {128, 50, 80, 15},
    // HIGH
    {255, 50, 100, 25}};

// Strategy enumeration
enum Strategy
{
    STRATEGY_STING = 0,
    STRATEGY_SPEED = 1,
    STRATEGY_RUN = 2,
    STRATEGY_IMU_HOLD = 3,
    STRATEGY_COUNT = 4
};

enum StartRoutine
{
    START_ROUTINE_STRAIGHT = 0,
    START_ROUTINE_COUNT = 1
};

// Motor direction enumeration
enum MotorDirection
{
    DIRECTION_FORWARD = 0,
    DIRECTION_BACKWARD = 1,
    DIRECTION_LEFT = 2,
    DIRECTION_RIGHT = 3,
    DIRECTION_STOP = 4
};

#endif // MENU_H
