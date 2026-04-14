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

private:
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

    void updateBehavior();
    void updateBehavior_Speed();
    void updateBehavior_Sting();
    void updateBehavior_Run();
    void updateBehavior_IMUHold();

    void applySpeedPreset(int level);
    void cycleMenuScreenBackward();
    void cycleSpeedLevelBackward();
    void cycleStrategyBackward();
};

#endif // ROBOT_H
