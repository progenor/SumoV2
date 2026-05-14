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
#include "HeadingController.h"

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
    int getCurrentLeftMotorPWM() const;
    int getCurrentRightMotorPWM() const;
    bool isDiagnosticsMotorTestActive() const;
    int getDiagnosticsMotorTestSelection() const;

    void handleKeypadAction(KeypadAction action);

    float getBatteryVoltage();
    int getBatteryRawAdc();
    float getBatteryAdcVoltageFromRaw(int rawAdc);
    float getBatteryVoltageFromRaw(int rawAdc);
    float getTemperatureVoltage();
    float getTemperatureC();

private:
    void setMotorPWM(int leftPWM, int rightPWM);
    enum IMUState
    {
        IMU_STATE_START_DELAY,
        IMU_STATE_START_ROUTINE,
        IMU_STATE_SEARCH,
        IMU_STATE_ATTACK,
        IMU_STATE_EVASION,
        IMU_STATE_EDGE_RECOVERY
    };

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
    HeadingController headingController;

    RobotMode currentMode;
    int currentMenuScreen;
    bool paused;
    int currentSpeedLevel;
    int currentStrategy;
    int currentMotorDirection;
    int lastLeftMotorPWM;
    int lastRightMotorPWM;

    BuzzerMode buzzerMode;
    bool buzzerOutputOn;
    bool batterySampleValid;
    float lastBatteryVoltage;
    unsigned long buzzerLastToggleMs;
    int buzzerTransitionsRemaining;

    bool imuAttackLocked;
    unsigned long imuAttackRecoilStartMs;
    float imuAttackHeadingDeg;
    unsigned long imuPhaseStartMs;
    int imuEvasionStep;
    IMUState imuState;
    int previousStrategy;
    unsigned long imuStartRoutineStartMs;
    unsigned long imuStartDelayStartMs;
    unsigned long imuTargetLostMs;
    bool imuTargetPreviouslySeen;
    float imuLastHeadingErrorDeg;
    float imuLastPidCorrection;
    bool imuAvailable;
    unsigned long imuLastPrintMs;

    bool adcCacheValid;
    unsigned long adcLastSampleMs;
    int cachedBatteryRawAdc;
    float cachedBatteryAdcVoltage;
    float cachedBatteryVoltage;
    float cachedTemperatureVoltage;
    float cachedTemperatureC;

    bool qtrLineSensorsEnabled;
    int imuLastSeenDirection;
    int imuSearchDirection;
    unsigned long imuSearchPhaseStartMs;
    bool imuSearchForwardPulse;
    unsigned long stingRightCommitUntilMs;
    int stingCommittedTurnDirection;

    bool diagnosticsMotorTestActive;
    int diagnosticsMotorTestSelection;

    void updateBehavior();
    void updateBehavior_Speed();
    void updateBehavior_Sting();
    void updateBehavior_Run();
    void updateBehavior_IMUHold();
    void updateBehavior_DiagnosticsMotorTest();
    void resetIMUStrategyState();
    void updateIMUStateMachine(int *irValues, int *qtrValues, unsigned long nowMs);
    void runIMUStartDelay(unsigned long nowMs);
    void runIMUStartRoutine(unsigned long nowMs);
    void runIMUSearch(int *irValues);
    void runIMUAttack(int *irValues, unsigned long nowMs);
    void runIMUEvasion(unsigned long nowMs);
    void runIMUEdgeRecovery(int *qtrValues, unsigned long nowMs);
    unsigned long getSelectedStartDelayMs() const;
    void beginIMUSearchPhase(unsigned long nowMs);

    void applySpeedPreset(int level);
    void cycleMenuScreenBackward();
    void cycleSpeedLevelBackward();
    void cycleStrategyBackward();
    void cycleStartDelayBackward();

    void enterDiagnosticsMotorTest();
    void exitDiagnosticsMotorTest();
    void cycleDiagnosticsMotorTest();
    void cycleDiagnosticsMotorTestBackward();

    void updateBatteryBuzzer();
    void setBuzzerOutput(bool on);
    void startWarningPattern();
    void startContinuousAlarm();
    void stopBuzzerAlarm();

    int sampleAveragedAdc(uint8_t pin);
    void refreshAdcCache(unsigned long nowMs);
};

#endif // ROBOT_H
