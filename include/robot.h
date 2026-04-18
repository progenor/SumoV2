#ifndef ROBOT_H
#define ROBOT_H

#include <Arduino.h>
#include "motors.h"
#include "display.h"
#include "ir.h"
#include "qtr.h"
#include "melody.h"
#include "defines.h"
#include "menu.h"
#include "button.h"
#include "IMU.h"

class Robot
{
public:
    Robot();

    void setup();
    void update();
    void testDirections();

    SpeedConfig &getSpeedConfig();
    int *getIRValues();
    Display &getDisplay();
    Motor &getMotor();

    RobotMode getMode() const;
    void setMode(RobotMode mode);

    int getCurrentMenuScreen() const;
    void setCurrentMenuScreen(int screen);
    void cycleMenuScreen();

    bool isPaused() const;
    void togglePause();

    int getCurrentSpeedLevel() const;
    void setSpeedLevel(int level);
    void cycleSpeedLevel();

    int getCurrentStrategy() const;
    void setStrategy(int strategy);
    void cycleStrategy();

    int getCurrentDirection() const;

    void handleKeypadAction(KeypadAction action);

    float getBatteryVoltage();
    int getBatteryRawAdc();
    float getBatteryAdcVoltageFromRaw(int rawAdc);
    float getBatteryVoltageFromRaw(int rawAdc);
    float getTemperatureVoltage();
    float getTemperatureC();

private:
    enum BuzzerMode
    {
        BUZZER_MODE_IDLE,
        BUZZER_MODE_WARN_PATTERN,
        BUZZER_MODE_CONTINUOUS
    };

    Motor motor;
    Display display;
    IRSensors irSensors;
    QTRSensors qtrSensors;
    IMU imu;

    RobotMode currentMode;
    int currentMenuScreen;
    bool paused;
    int currentSpeedLevel;
    int currentStrategy;
    int currentMotorDirection;

    BuzzerMode buzzerMode;
    bool buzzerOutputOn;
    bool batterySampleValid;
    float lastBatteryVoltage;
    unsigned long buzzerLastToggleMs;
    int buzzerTransitionsRemaining;

    void updateBehavior();
    void updateBehavior_Speed();
    void updateBehavior_Sting();
    void updateBehavior_Run();
    void updateBehavior_IMUHold();

    void applySpeedPreset(int level);
    void cycleMenuScreenBackward();
    void cycleSpeedLevelBackward();
    void cycleStrategyBackward();

    void updateBatteryBuzzer();
    void setBuzzerOutput(bool on);
    void startWarningPattern();
    void startContinuousAlarm();
    void stopBuzzerAlarm();
};

#endif // ROBOT_H
