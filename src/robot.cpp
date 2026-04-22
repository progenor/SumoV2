#include "robot.h"
#include "pins.h"
#include <math.h>

SpeedConfig speedConfig;

static const unsigned long IMU_START_ROUTINE_MS = 550;
static const unsigned long IMU_TARGET_LOST_EVASION_DELAY_MS = 120;
static const unsigned long IMU_EVASION_BACKUP_MS = 220;
static const unsigned long IMU_EVASION_TURN_MS = 160;
static const unsigned long IMU_SEARCH_PHASE_MS = 220;
static const unsigned long IMU_SEARCH_PULSE_MS = 80;
static const unsigned long IMU_ATTACK_RECOIL_MS = 40;
static const unsigned long STING_TURN_COMMIT_MS = 50;

Robot::Robot()
    : currentMode(MODE_MENU),
      currentMenuScreen(MENU_SCREEN_MAIN),
      paused(false),
      currentSpeedLevel(SPEED_LEVEL_LOW),
      currentStrategy(STRATEGY_STING),
      currentStartRoutine(START_ROUTINE_STRAIGHT),
      currentMotorDirection(DIRECTION_STOP),
      lastLeftMotorPWM(0),
      lastRightMotorPWM(0),
      buzzerMode(BUZZER_MODE_IDLE),
      buzzerOutputOn(false),
      batterySampleValid(false),
      lastBatteryVoltage(0.0f),
      buzzerLastToggleMs(0),
      buzzerTransitionsRemaining(0),
      imuAttackLocked(false),
      imuAttackRecoilStartMs(0),
      imuAttackHeadingDeg(0.0f),
      imuPhaseStartMs(0),
      imuEvasionStep(0),
      imuState(IMU_STATE_START_ROUTINE),
      previousStrategy(STRATEGY_STING),
      imuStartRoutineStartMs(0),
      imuStartDelayStartMs(0),
      imuTargetLostMs(0),
      imuTargetPreviouslySeen(false),
      imuLastHeadingErrorDeg(0.0f),
      imuLastPidCorrection(0.0f),
      imuAvailable(false),
      imuLastPrintMs(0),
      adcCacheValid(false),
      adcLastSampleMs(0),
      cachedBatteryRawAdc(0),
      cachedBatteryAdcVoltage(0.0f),
      cachedBatteryVoltage(0.0f),
      cachedTemperatureVoltage(0.0f),
      cachedTemperatureC(NAN),
      qtrLineSensorsEnabled(ENABLE_QTR_LINE_SENSORS != 0),
      imuLastSeenDirection(1),
      imuSearchDirection(1),
      imuSearchPhaseStartMs(0),
      imuSearchForwardPulse(false),
      stingRightCommitUntilMs(0),
      stingCommittedTurnDirection(1)
{
}

void Robot::setup()
{
    Serial.begin(115200);
    unsigned long serialWaitStart = millis();
    while (!Serial && (millis() - serialWaitStart) < 300)
    {
        delay(1);
    }

#if DEBUG_ROBOT_BOOT
    Serial.println("[BOOT] Robot setup starting...");
#endif

    Wire.begin();
    Wire.setClock(400000);
    setupPins();

    display.setup();
    display.drawLoadingScreen();

    motor.setup();
    irSensors.setup();
    if (qtrLineSensorsEnabled)
    {
        qtrSensors.setup();
    }

    imuAvailable = imu.begin();
    if (imuAvailable)
    {
        imu.calibrateGyro();
        imu.resetHeading();
#if DEBUG_ROBOT_BOOT
        Serial.println("IMU initialized successfully.");
#endif
    }
    else
    {
#if DEBUG_ROBOT_BOOT
        Serial.println("IMU init failed. Continuing without IMU updates.");
#endif
    }
    headingController.reset();
    previousStrategy = currentStrategy;
    resetIMUStrategyState();
    applySpeedPreset(currentSpeedLevel);

    // playMelody();
}

void Robot::update()
{
    unsigned long nowMs = millis();
    refreshAdcCache(nowMs);

    irSensors.read();
    if (qtrLineSensorsEnabled)
    {
        qtrSensors.read();
    }
    if (imuAvailable)
    {
        imu.read();

#if DEBUG_IMU_RUNTIME
        if ((nowMs - imuLastPrintMs) >= 250)
        {
            Serial.print("[IMU] ");
            imu.printData();
            imuLastPrintMs = nowMs;
        }
#endif
    }

    updateBehavior();
    updateBatteryBuzzer();

    if (currentMode != MODE_MENU)
    {
        display.displayIR(getIRValues(), IRCount);
    }
}

void Robot::testDirections()
{
    motor.testDirections();
}

void Robot::setBuzzerOutput(bool on)
{
    if (on == buzzerOutputOn)
    {
        return;
    }

    if (on)
    {
        tone(BUZZER, 2200);
    }
    else
    {
        noTone(BUZZER);
    }
    buzzerOutputOn = on;
}

void Robot::setMotorPWM(int leftPWM, int rightPWM)
{
    lastLeftMotorPWM = leftPWM;
    lastRightMotorPWM = rightPWM;
}

void Robot::startWarningPattern()
{
    buzzerMode = BUZZER_MODE_WARN_PATTERN;
    // Start ON immediately, then do 5 transitions every 500ms:
    // ON->OFF->ON->OFF->ON->OFF  => 3 beeps, each 500ms.
    buzzerTransitionsRemaining = 5;
    buzzerLastToggleMs = millis();
    setBuzzerOutput(true);
}

void Robot::startContinuousAlarm()
{
    buzzerMode = BUZZER_MODE_CONTINUOUS;
    buzzerTransitionsRemaining = 0;
    setBuzzerOutput(true);
}

void Robot::stopBuzzerAlarm()
{
    buzzerMode = BUZZER_MODE_IDLE;
    buzzerTransitionsRemaining = 0;
    setBuzzerOutput(false);
}

void Robot::updateBatteryBuzzer()
{
    const float usbFloorV = 6.0f;
    const float warningThresholdV = 12.2f;
    const float criticalThresholdV = 11.3f;

    float currentBatteryV = getBatteryVoltage();

    if (currentBatteryV < usbFloorV)
    {
        stopBuzzerAlarm();
        batterySampleValid = false;
        lastBatteryVoltage = currentBatteryV;
        return;
    }

    if (!batterySampleValid)
    {
        batterySampleValid = true;
        lastBatteryVoltage = currentBatteryV;
        return;
    }

    bool crossedBelowWarning = (lastBatteryVoltage >= warningThresholdV) && (currentBatteryV < warningThresholdV);

    if (currentBatteryV < criticalThresholdV)
    {
        if (buzzerMode != BUZZER_MODE_CONTINUOUS)
        {
            startContinuousAlarm();
        }
    }
    else
    {
        if (buzzerMode == BUZZER_MODE_CONTINUOUS)
        {
            stopBuzzerAlarm();
        }

        if (crossedBelowWarning && buzzerMode == BUZZER_MODE_IDLE)
        {
            startWarningPattern();
        }
    }

    if (buzzerMode == BUZZER_MODE_WARN_PATTERN)
    {
        unsigned long now = millis();
        if ((now - buzzerLastToggleMs) >= 500)
        {
            buzzerLastToggleMs = now;
            setBuzzerOutput(!buzzerOutputOn);
            buzzerTransitionsRemaining--;

            if (buzzerTransitionsRemaining <= 0)
            {
                stopBuzzerAlarm();
            }
        }
    }

    lastBatteryVoltage = currentBatteryV;
}

void Robot::updateBehavior_Speed()
{
    if (paused)
    {
        motor.stop();
        currentMotorDirection = DIRECTION_STOP;
        return;
    }

    int *irValues = irSensors.getAllValues();

    if (irValues[1] == 1)
    {
        motor.forward(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_FORWARD;
    }
    else if (irValues[0] == 1)
    {
        motor.left(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_LEFT;
    }
    else if (irValues[2] == 1)
    {
        motor.right(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_RIGHT;
    }
    else
    {
        motor.right(speedConfig.search_speed);
        currentMotorDirection = DIRECTION_RIGHT;
    }
}

void Robot::updateBehavior_Sting()
{
    if (paused)
    {
        motor.stop();
        currentMotorDirection = DIRECTION_STOP;
        stingRightCommitUntilMs = 0;
        stingCommittedTurnDirection = 1;
        return;
    }

    unsigned long nowMs = millis();
    int *irValues = irSensors.getAllValues();

    if (irValues[1] == 1)
    {
        stingRightCommitUntilMs = 0;
        stingCommittedTurnDirection = 1;
        motor.forward(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_FORWARD;
    }
    else if (nowMs < stingRightCommitUntilMs)
    {
        if (stingCommittedTurnDirection < 0)
        {
            motor.left(speedConfig.turn_speed_moderate);
            currentMotorDirection = DIRECTION_LEFT;
        }
        else
        {
            motor.right(speedConfig.turn_speed_moderate);
            currentMotorDirection = DIRECTION_RIGHT;
        }
    }
    else if (irValues[0] == 1)
    {
        stingCommittedTurnDirection = -1;
        stingRightCommitUntilMs = nowMs + STING_TURN_COMMIT_MS;
        motor.left(speedConfig.turn_speed_moderate);
        currentMotorDirection = DIRECTION_LEFT;
    }
    else if (irValues[2] == 1)
    {
        stingCommittedTurnDirection = 1;
        stingRightCommitUntilMs = nowMs + STING_TURN_COMMIT_MS;
        motor.right(speedConfig.turn_speed_moderate);
        currentMotorDirection = DIRECTION_RIGHT;
    }
    else
    {
        motor.right(speedConfig.search_speed);
        currentMotorDirection = DIRECTION_RIGHT;
    }
}

void Robot::updateBehavior_Run()
{
    if (paused)
    {
        motor.stop();
        currentMotorDirection = DIRECTION_STOP;
        return;
    }

    int *irValues = irSensors.getAllValues();

    if (irValues[1] == 1)
    {
        motor.backward(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_BACKWARD;
    }
    else if (irValues[0] == 1)
    {
        motor.right(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_RIGHT;
    }
    else if (irValues[2] == 1)
    {
        motor.left(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_LEFT;
    }
    else
    {
        motor.left(speedConfig.search_speed);
        currentMotorDirection = DIRECTION_LEFT;
    }
}

void Robot::updateBehavior_IMUHold()
{
    if (paused)
    {
        motor.stop();
        currentMotorDirection = DIRECTION_STOP;
        return;
    }

    int *irValues = irSensors.getAllValues();
    int *qtrValues = qtrLineSensorsEnabled ? qtrSensors.getAllValues() : nullptr;
    unsigned long nowMs = millis();

    updateIMUStateMachine(irValues, qtrValues, nowMs);
}

void Robot::resetIMUStrategyState()
{
    imuAttackLocked = false;
    imuAttackRecoilStartMs = 0;
    imuAttackHeadingDeg = imu.getHeadingDeg();
    imuPhaseStartMs = 0;
    imuEvasionStep = 0;
    imuState = IMU_STATE_START_DELAY;
    imuStartDelayStartMs = millis();
    imuStartRoutineStartMs = millis();
    imuTargetLostMs = 0;
    imuTargetPreviouslySeen = false;
    imuLastHeadingErrorDeg = 0.0f;
    imuLastPidCorrection = 0.0f;
    imuLastSeenDirection = 1;
    imuSearchDirection = 1;
    imuSearchPhaseStartMs = millis();
    imuSearchForwardPulse = false;
    headingController.reset();
}

void Robot::beginIMUSearchPhase(unsigned long nowMs)
{
    imuSearchDirection = (imuLastSeenDirection < 0) ? -1 : 1;
    imuSearchForwardPulse = false;
    imuSearchPhaseStartMs = nowMs;
}

void Robot::updateIMUStateMachine(int *irValues, int *qtrValues, unsigned long nowMs)
{
    bool edgeLeft = false;
    bool edgeRight = false;
    bool edgeDetected = false;
    if (qtrLineSensorsEnabled && qtrValues != nullptr)
    {
        edgeLeft = (qtrValues[0] == 1);
        edgeRight = (qtrValues[1] == 1);
        edgeDetected = edgeLeft || edgeRight;
    }
    bool centerDetected = (irValues[1] == 1);

    if (edgeDetected)
    {
        imuState = IMU_STATE_EDGE_RECOVERY;
        imuEvasionStep = edgeLeft && !edgeRight ? 100 : (edgeRight && !edgeLeft ? 101 : 102);
        imuPhaseStartMs = nowMs;
        imuAttackLocked = false;
        headingController.reset();
    }

    switch (imuState)
    {
    case IMU_STATE_START_DELAY:
        if (centerDetected)
        {
            imuState = IMU_STATE_ATTACK;
            imuAttackLocked = false;
            imuLastSeenDirection = 0;
        }
        else
        {
            runIMUStartDelay(nowMs);
        }
        break;

    case IMU_STATE_START_ROUTINE:
        if (centerDetected)
        {
            imuState = IMU_STATE_ATTACK;
            imuAttackLocked = false;
            imuLastSeenDirection = 0;
        }
        else
        {
            runIMUStartRoutine(nowMs);
        }
        break;

    case IMU_STATE_SEARCH:
        if (centerDetected)
        {
            imuState = IMU_STATE_ATTACK;
            imuAttackLocked = false;
            imuLastSeenDirection = 0;
        }
        else
        {
            runIMUSearch(irValues);
        }
        break;

    case IMU_STATE_ATTACK:
        runIMUAttack(irValues, nowMs);
        break;

    case IMU_STATE_EVASION:
        runIMUEvasion(nowMs);
        break;

    case IMU_STATE_EDGE_RECOVERY:
        runIMUEdgeRecovery(qtrValues, nowMs);
        break;

    default:
        imuState = IMU_STATE_SEARCH;
        break;
    }
}

void Robot::runIMUStartDelay(unsigned long nowMs)
{
    // Delay removed - transition immediately to start routine
    motor.stop();
    setMotorPWM(0, 0);
    currentMotorDirection = DIRECTION_STOP;
    imuState = IMU_STATE_START_ROUTINE;
    imuStartRoutineStartMs = nowMs;
}

void Robot::runIMUStartRoutine(unsigned long nowMs)
{
    unsigned long elapsedMs = nowMs - imuStartRoutineStartMs;
    if (elapsedMs >= IMU_START_ROUTINE_MS)
    {
        imuState = IMU_STATE_SEARCH;
        beginIMUSearchPhase(nowMs);
        return;
    }

    switch (currentStartRoutine)
    {
    case START_ROUTINE_LEFT_ARC:
        motor.drive(speedConfig.search_speed - 20, speedConfig.search_speed + 10, true, false);
        setMotorPWM(speedConfig.search_speed - 20, speedConfig.search_speed + 10);
        currentMotorDirection = DIRECTION_FORWARD;
        break;
    case START_ROUTINE_RIGHT_ARC:
        motor.drive(speedConfig.search_speed + 10, speedConfig.search_speed - 20, true, false);
        setMotorPWM(speedConfig.search_speed + 10, speedConfig.search_speed - 20);
        currentMotorDirection = DIRECTION_FORWARD;
        break;
    case START_ROUTINE_SPIN_WAIT:
        motor.drive(speedConfig.turn_speed_moderate, speedConfig.turn_speed_moderate, true, true);
        setMotorPWM(speedConfig.turn_speed_moderate, speedConfig.turn_speed_moderate);
        currentMotorDirection = DIRECTION_RIGHT;
        break;
    case START_ROUTINE_STRAIGHT:
    default:
        motor.drive(speedConfig.search_speed, speedConfig.search_speed, true, false);
        setMotorPWM(speedConfig.search_speed, speedConfig.search_speed);
        currentMotorDirection = DIRECTION_FORWARD;
        break;
    }
}

void Robot::runIMUSearch(int *irValues)
{
    imuAttackLocked = false;
    imuLastHeadingErrorDeg = 0.0f;
    imuLastPidCorrection = 0.0f;

    if (irValues[0] == 1)
    {
        imuLastSeenDirection = -1;
        motor.left(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_LEFT;
        return;
    }

    if (irValues[2] == 1)
    {
        imuLastSeenDirection = 1;
        motor.right(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_RIGHT;
        return;
    }

    unsigned long nowMs = millis();
    unsigned long phaseElapsed = nowMs - imuSearchPhaseStartMs;
    if (phaseElapsed >= IMU_SEARCH_PHASE_MS)
    {
        imuSearchForwardPulse = !imuSearchForwardPulse;
        imuSearchPhaseStartMs = nowMs;
        if (!imuSearchForwardPulse)
        {
            imuSearchDirection = -imuSearchDirection;
        }
        phaseElapsed = 0;
    }

    if (imuSearchForwardPulse && phaseElapsed < IMU_SEARCH_PULSE_MS)
    {
        motor.forward(speedConfig.search_speed);
        setMotorPWM(speedConfig.search_speed, speedConfig.search_speed);
        currentMotorDirection = DIRECTION_FORWARD;
        return;
    }

    int searchTurnPwm = speedConfig.turn_speed_moderate;
    if (searchTurnPwm < speedConfig.search_speed)
    {
        searchTurnPwm = speedConfig.search_speed;
    }

    if (imuSearchDirection < 0)
    {
        motor.drive(searchTurnPwm, searchTurnPwm, false, false);
        setMotorPWM(searchTurnPwm, searchTurnPwm);
        currentMotorDirection = DIRECTION_LEFT;
    }
    else
    {
        motor.drive(searchTurnPwm, searchTurnPwm, true, true);
        setMotorPWM(searchTurnPwm, searchTurnPwm);
        currentMotorDirection = DIRECTION_RIGHT;
    }
}

void Robot::runIMUAttack(int *irValues, unsigned long nowMs)
{
    if (irValues[1] == 1)
    {
        imuTargetPreviouslySeen = true;
        imuTargetLostMs = 0;
        imuLastSeenDirection = 0;

        if (!imuAttackLocked)
        {
            if (imuAttackRecoilStartMs == 0)
            {
                imuAttackRecoilStartMs = nowMs;
            }

            if ((nowMs - imuAttackRecoilStartMs) < IMU_ATTACK_RECOIL_MS)
            {
                motor.backward(speedConfig.attack_speed);
                setMotorPWM(speedConfig.attack_speed, speedConfig.attack_speed);
                currentMotorDirection = DIRECTION_BACKWARD;
                return;
            }
        }

        imuAttackRecoilStartMs = 0;
        if (!imuAttackLocked)
        {
            imuAttackHeadingDeg = imu.getHeadingDeg();
            headingController.reset();
            imuAttackLocked = true;
        }

        float headingError = imuAttackHeadingDeg - imu.getHeadingDeg();
        float correction = headingController.update(headingError, nowMs);
        imuLastHeadingErrorDeg = headingError;
        imuLastPidCorrection = correction;
        int leftPwm = constrain(static_cast<int>(speedConfig.attack_speed - correction), 0, 255);
        int rightPwm = constrain(static_cast<int>(speedConfig.attack_speed + correction), 0, 255);
        motor.drive(leftPwm, rightPwm, true, false);
        setMotorPWM(leftPwm, rightPwm);
        currentMotorDirection = DIRECTION_FORWARD;
        return;
    }

    imuAttackLocked = false;
    imuAttackRecoilStartMs = 0;
    imuLastHeadingErrorDeg = 0.0f;
    imuLastPidCorrection = 0.0f;
    if (imuTargetLostMs == 0)
    {
        imuTargetLostMs = nowMs;
    }

    if (irValues[0] == 1)
    {
        imuLastSeenDirection = -1;
        motor.left(speedConfig.attack_speed);
        setMotorPWM(speedConfig.attack_speed, speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_LEFT;
        return;
    }

    if (irValues[2] == 1)
    {
        imuLastSeenDirection = 1;
        motor.right(speedConfig.attack_speed);
        setMotorPWM(speedConfig.attack_speed, speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_RIGHT;
        return;
    }

    if (imuTargetPreviouslySeen && (nowMs - imuTargetLostMs) >= IMU_TARGET_LOST_EVASION_DELAY_MS)
    {
        imuState = IMU_STATE_EVASION;
        imuEvasionStep = 200;
        imuPhaseStartMs = nowMs;
        imuTargetPreviouslySeen = false;
        return;
    }

    imuState = IMU_STATE_SEARCH;
    beginIMUSearchPhase(nowMs);
}

void Robot::runIMUEvasion(unsigned long nowMs)
{
    imuLastHeadingErrorDeg = 0.0f;
    imuLastPidCorrection = 0.0f;

    unsigned long elapsedMs = nowMs - imuPhaseStartMs;

    if (imuEvasionStep == 200)
    {
        motor.backward(speedConfig.attack_speed);
        setMotorPWM(speedConfig.attack_speed, speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_BACKWARD;
        if (elapsedMs >= IMU_EVASION_BACKUP_MS)
        {
            imuEvasionStep = 201;
            imuPhaseStartMs = nowMs;
        }
        return;
    }

    if (imuEvasionStep == 201)
    {
        motor.drive(speedConfig.turn_speed_moderate, speedConfig.turn_speed_moderate, true, true);
        setMotorPWM(speedConfig.turn_speed_moderate, speedConfig.turn_speed_moderate);
        currentMotorDirection = DIRECTION_RIGHT;
        if (elapsedMs >= IMU_EVASION_TURN_MS)
        {
            imuEvasionStep = 202;
            imuPhaseStartMs = nowMs;
        }
        return;
    }

    if (imuEvasionStep == 202)
    {
        motor.forward(speedConfig.search_speed);
        setMotorPWM(speedConfig.search_speed, speedConfig.search_speed);
        currentMotorDirection = DIRECTION_FORWARD;
        if (elapsedMs >= 120)
        {
            imuState = IMU_STATE_SEARCH;
            imuEvasionStep = 0;
            imuTargetLostMs = 0;
            beginIMUSearchPhase(nowMs);
        }
        return;
    }

    imuState = IMU_STATE_SEARCH;
    imuEvasionStep = 0;
    beginIMUSearchPhase(nowMs);
}

void Robot::runIMUEdgeRecovery(int *qtrValues, unsigned long nowMs)
{
    imuLastHeadingErrorDeg = 0.0f;
    imuLastPidCorrection = 0.0f;

    unsigned long elapsedMs = nowMs - imuPhaseStartMs;

    if (imuEvasionStep == 100 || imuEvasionStep == 101 || imuEvasionStep == 102)
    {
        motor.backward(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_BACKWARD;
        if (elapsedMs >= IMU_EVASION_BACKUP_MS)
        {
            imuEvasionStep = (imuEvasionStep == 100) ? 103 : ((imuEvasionStep == 101) ? 104 : 105);
            imuPhaseStartMs = nowMs;
        }
        return;
    }

    if (imuEvasionStep == 103)
    {
        motor.drive(speedConfig.turn_speed_moderate, speedConfig.turn_speed_moderate, true, true);
        currentMotorDirection = DIRECTION_RIGHT;
        if (elapsedMs >= IMU_EVASION_TURN_MS)
        {
            imuState = IMU_STATE_SEARCH;
            imuEvasionStep = 0;
            imuTargetLostMs = 0;
            beginIMUSearchPhase(nowMs);
        }
        return;
    }

    if (imuEvasionStep == 104)
    {
        motor.drive(speedConfig.turn_speed_moderate, speedConfig.turn_speed_moderate, false, false);
        setMotorPWM(speedConfig.turn_speed_moderate, speedConfig.turn_speed_moderate);
        currentMotorDirection = DIRECTION_LEFT;
        if (elapsedMs >= IMU_EVASION_TURN_MS)
        {
            imuState = IMU_STATE_SEARCH;
            imuEvasionStep = 0;
            imuTargetLostMs = 0;
            beginIMUSearchPhase(nowMs);
        }
        return;
    }

    if (imuEvasionStep == 105)
    {
        // When both edge sensors trigger, keep reversing until one side clears.
        if (qtrValues == nullptr || (qtrValues[0] == 0 && qtrValues[1] == 0))
        {
            imuState = IMU_STATE_SEARCH;
            imuEvasionStep = 0;
            imuTargetLostMs = 0;
            beginIMUSearchPhase(nowMs);
            return;
        }
        motor.backward(speedConfig.attack_speed);
        setMotorPWM(speedConfig.attack_speed, speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_BACKWARD;
        return;
    }

    imuState = IMU_STATE_SEARCH;
    imuEvasionStep = 0;
    beginIMUSearchPhase(nowMs);
}

void Robot::updateBehavior()
{
    if (currentStrategy != previousStrategy)
    {
        if (currentStrategy == STRATEGY_IMU_HOLD)
        {
            resetIMUStrategyState();
        }
        previousStrategy = currentStrategy;
    }

    switch (currentStrategy)
    {
    case STRATEGY_STING:
        updateBehavior_Sting();
        break;
    case STRATEGY_SPEED:
        updateBehavior_Speed();
        break;
    case STRATEGY_RUN:
        updateBehavior_Run();
        break;
    case STRATEGY_IMU_HOLD:
        updateBehavior_IMUHold();
        break;
    default:
        updateBehavior_Speed();
        break;
    }
}

void Robot::handleKeypadAction(KeypadAction action)
{
    switch (action)
    {
    case KEYPAD_ACTION_H:
        if (currentMode == MODE_MENU)
        {
            cycleMenuScreenBackward();
        }
        break;

    case KEYPAD_ACTION_L:
        if (currentMode == MODE_MENU)
        {
            cycleMenuScreen();
        }
        break;

    case KEYPAD_ACTION_J:
        if (currentMenuScreen == MENU_SCREEN_SPEED)
        {
            cycleSpeedLevelBackward();
        }
        else if (currentMenuScreen == MENU_SCREEN_STRATEGY)
        {
            cycleStrategyBackward();
        }
        else if (currentMenuScreen == MENU_SCREEN_START_ROUTINE)
        {
            cycleStartRoutineBackward();
        }
        break;

    case KEYPAD_ACTION_K:
        if (currentMenuScreen == MENU_SCREEN_SPEED)
        {
            cycleSpeedLevel();
        }
        else if (currentMenuScreen == MENU_SCREEN_STRATEGY)
        {
            cycleStrategy();
        }
        else if (currentMenuScreen == MENU_SCREEN_START_ROUTINE)
        {
            cycleStartRoutine();
        }
        break;

    case KEYPAD_ACTION_NONE:
    default:
        break;
    }
}

void Robot::applySpeedPreset(int level)
{
    if (level >= 0 && level < SPEED_LEVEL_COUNT)
    {
        const SpeedPreset &preset = SPEED_PRESETS[level];
        speedConfig.attack_speed = preset.attack;
        speedConfig.search_speed = preset.search;
        speedConfig.turn_speed_moderate = preset.turn_moderate;
        speedConfig.turn_speed_gentle = preset.turn_gentle;
    }
}

SpeedConfig &Robot::getSpeedConfig()
{
    return speedConfig;
}

int *Robot::getIRValues()
{
    return irSensors.getAllValues();
}

Display &Robot::getDisplay()
{
    return display;
}

Motor &Robot::getMotor()
{
    return motor;
}

RobotMode Robot::getMode() const
{
    return currentMode;
}

void Robot::setMode(RobotMode mode)
{
    currentMode = mode;
}

int Robot::getCurrentMenuScreen() const
{
    return currentMenuScreen;
}

void Robot::setCurrentMenuScreen(int screen)
{
    currentMenuScreen = screen % MENU_SCREEN_COUNT;
}

void Robot::cycleMenuScreen()
{
    currentMenuScreen = (currentMenuScreen + 1) % MENU_SCREEN_COUNT;
}

bool Robot::isPaused() const
{
    return paused;
}

void Robot::togglePause()
{
    paused = !paused;
    currentMode = paused ? MODE_PAUSED : MODE_MENU;
    if (paused)
    {
        motor.stop();
    }
}

int Robot::getCurrentSpeedLevel() const
{
    return currentSpeedLevel;
}

void Robot::setSpeedLevel(int level)
{
    if (level >= 0 && level < SPEED_LEVEL_COUNT)
    {
        currentSpeedLevel = level;
        applySpeedPreset(level);
    }
}

void Robot::cycleSpeedLevel()
{
    setSpeedLevel((currentSpeedLevel + 1) % SPEED_LEVEL_COUNT);
}

int Robot::getCurrentStrategy() const
{
    return currentStrategy;
}

int Robot::getCurrentStartRoutine() const
{
    return currentStartRoutine;
}

int Robot::getCurrentDirection() const
{
    return currentMotorDirection;
}

int Robot::getCurrentLeftMotorPWM() const
{
    return lastLeftMotorPWM;
}

int Robot::getCurrentRightMotorPWM() const
{
    return lastRightMotorPWM;
}

float Robot::getBatteryVoltage()
{
    if (!adcCacheValid)
    {
        refreshAdcCache(millis());
    }
    return cachedBatteryVoltage;
}

int Robot::getBatteryRawAdc()
{
    if (!adcCacheValid)
    {
        refreshAdcCache(millis());
    }
    return cachedBatteryRawAdc;
}

float Robot::getBatteryAdcVoltageFromRaw(int rawAdc)
{
    return (static_cast<float>(rawAdc) / 4095.0f) * 3.3f;
}

float Robot::getBatteryVoltageFromRaw(int rawAdc)
{
    // Divider: R25=56k (top), R26=10k (bottom), Vadc = Vbat * (10 / 66)
    const float dividerScale = (56.0f + 10.0f) / 10.0f; // 6.6

    float vAdc = getBatteryAdcVoltageFromRaw(rawAdc);
    // float correctedAdc = vAdc - BATTERY_ADC_OFFSET_V;
    float correctedAdc = vAdc + 0.12f; // Adjusted to match measured values better
    if (correctedAdc < 0.0f)
    {
        correctedAdc = 0.0f;
    }
    return correctedAdc * dividerScale;
}

float Robot::getTemperatureVoltage()
{
    if (!adcCacheValid)
    {
        refreshAdcCache(millis());
    }
    return cachedTemperatureVoltage;
}

float Robot::getTemperatureC()
{
    if (!adcCacheValid)
    {
        refreshAdcCache(millis());
    }
    return cachedTemperatureC;
}

int Robot::sampleAveragedAdc(uint8_t pin)
{
    const int sampleCount = 8;
    int sum = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        sum += analogRead(pin);
    }

    return sum / sampleCount;
}

void Robot::refreshAdcCache(unsigned long nowMs)
{
    if (adcCacheValid && (nowMs - adcLastSampleMs) < ADC_CACHE_INTERVAL_MS)
    {
        return;
    }

    cachedBatteryRawAdc = sampleAveragedAdc(BATTERY_LEVEL_PIN);
    cachedBatteryAdcVoltage = getBatteryAdcVoltageFromRaw(cachedBatteryRawAdc);
    cachedBatteryVoltage = getBatteryVoltageFromRaw(cachedBatteryRawAdc);

    int tempRawAdc = sampleAveragedAdc(TEMP_MONITOR_PIN);
    cachedTemperatureVoltage = (static_cast<float>(tempRawAdc) / 4095.0f) * 3.3f;

    float voltage = cachedTemperatureVoltage;

    if (!isfinite(voltage) || voltage <= 0.0f || voltage >= (TEMP_NTC_PULLUP_VOLTAGE - 0.01f))
    {
        cachedTemperatureC = NAN;
        adcLastSampleMs = nowMs;
        adcCacheValid = true;
        return;
    }

    float resistance = TEMP_NTC_PULLUP_RESISTANCE * voltage / (TEMP_NTC_PULLUP_VOLTAGE - voltage);
    if (!isfinite(resistance) || resistance <= 0.0f)
    {
        cachedTemperatureC = NAN;
        adcLastSampleMs = nowMs;
        adcCacheValid = true;
        return;
    }

    const float referenceKelvin = TEMP_NTC_REFERENCE_TEMP_C + 273.15f;
    float invTemperature = (1.0f / referenceKelvin) + (1.0f / TEMP_NTC_BETA) * logf(resistance / TEMP_NTC_R25);
    if (!isfinite(invTemperature) || invTemperature <= 0.0f)
    {
        cachedTemperatureC = NAN;
        adcLastSampleMs = nowMs;
        adcCacheValid = true;
        return;
    }

    cachedTemperatureC = (1.0f / invTemperature) - 273.15f;
    adcLastSampleMs = nowMs;
    adcCacheValid = true;
}

void Robot::cycleMenuScreenBackward()
{
    currentMenuScreen = (currentMenuScreen + MENU_SCREEN_COUNT - 1) % MENU_SCREEN_COUNT;
}

void Robot::cycleSpeedLevelBackward()
{
    setSpeedLevel((currentSpeedLevel + SPEED_LEVEL_COUNT - 1) % SPEED_LEVEL_COUNT);
}

void Robot::cycleStrategyBackward()
{
    setStrategy((currentStrategy + STRATEGY_COUNT - 1) % STRATEGY_COUNT);
}

void Robot::setStrategy(int strategy)
{
    if (strategy >= 0 && strategy < STRATEGY_COUNT)
    {
        currentStrategy = strategy;
    }
}

void Robot::cycleStrategy()
{
    setStrategy((currentStrategy + 1) % STRATEGY_COUNT);
}

void Robot::setStartRoutine(int startRoutine)
{
    if (startRoutine >= 0 && startRoutine < START_ROUTINE_COUNT)
    {
        currentStartRoutine = startRoutine;
    }
}

void Robot::cycleStartRoutine()
{
    setStartRoutine((currentStartRoutine + 1) % START_ROUTINE_COUNT);
}

void Robot::cycleStartRoutineBackward()
{
    setStartRoutine((currentStartRoutine + START_ROUTINE_COUNT - 1) % START_ROUTINE_COUNT);
}
