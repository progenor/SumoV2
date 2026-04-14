#include "robot.h"
#include "pins.h"
#include <math.h>

SpeedConfig speedConfig;

Robot::Robot()
    : currentMode(MODE_MENU),
      currentMenuScreen(MENU_SCREEN_MAIN),
      paused(false),
      currentSpeedLevel(SPEED_LEVEL_LOW),
      currentStrategy(STRATEGY_SPEED),
      currentMotorDirection(DIRECTION_STOP),
      buzzerMode(BUZZER_MODE_IDLE),
      buzzerOutputOn(false),
      batterySampleValid(false),
      lastBatteryVoltage(0.0f),
      buzzerLastToggleMs(0),
      buzzerTransitionsRemaining(0)
{
}

void Robot::setup()
{
    Serial.begin(115200);
    delay(500);

    Wire.begin();
    setupPins();

    display.setup();
    display.drawLoadingScreen();

    motor.setup();
    irSensors.setup();
    qtrSensors.setup();

    imu.begin();
    applySpeedPreset(currentSpeedLevel);

    playMelody();
}

void Robot::update()
{
    irSensors.read();
    qtrSensors.read();
    imu.read();

    updateBehavior();
    updateBatteryBuzzer();

    if (currentMode != MODE_MENU)
    {
        display.displayIR(getIRValues(), IRCount);
    }
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

    if (irValues[1] == 1)
    {
        motor.forward(speedConfig.attack_speed);
        currentMotorDirection = DIRECTION_FORWARD;
        return;
    }

    float gyroZ = imu.getGyroZ();
    if (!isnan(gyroZ) && gyroZ > 120.0f)
    {
        motor.left(speedConfig.turn_speed_moderate);
        currentMotorDirection = DIRECTION_LEFT;
    }
    else if (!isnan(gyroZ) && gyroZ < -120.0f)
    {
        motor.right(speedConfig.turn_speed_moderate);
        currentMotorDirection = DIRECTION_RIGHT;
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
        motor.forward(speedConfig.search_speed);
        currentMotorDirection = DIRECTION_FORWARD;
    }
}

void Robot::updateBehavior()
{
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

int Robot::getCurrentDirection() const
{
    return currentMotorDirection;
}

float Robot::getBatteryVoltage()
{
    return getBatteryVoltageFromRaw(getBatteryRawAdc());
}

int Robot::getBatteryRawAdc()
{
    const int sampleCount = 8;
    int sum = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        sum += analogRead(BATTERY_LEVEL_PIN);
    }

    return sum / sampleCount;
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
    float correctedAdc = vAdc - BATTERY_ADC_OFFSET_V;
    if (correctedAdc < 0.0f)
    {
        correctedAdc = 0.0f;
    }
    return correctedAdc * dividerScale;
}

float Robot::getTemperatureVoltage()
{
    const int sampleCount = 8;
    int sum = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        sum += analogRead(TEMP_MONITOR_PIN);
    }

    float rawAdc = static_cast<float>(sum) / sampleCount;
    return (rawAdc / 4095.0f) * 3.3f;
}

float Robot::getTemperatureC()
{
    float voltage = getTemperatureVoltage();

    if (!isfinite(voltage) || voltage <= 0.0f || voltage >= (TEMP_NTC_PULLUP_VOLTAGE - 0.01f))
    {
        return NAN;
    }

    float resistance = TEMP_NTC_PULLUP_RESISTANCE * voltage / (TEMP_NTC_PULLUP_VOLTAGE - voltage);
    if (!isfinite(resistance) || resistance <= 0.0f)
    {
        return NAN;
    }

    const float referenceKelvin = TEMP_NTC_REFERENCE_TEMP_C + 273.15f;
    float invTemperature = (1.0f / referenceKelvin) + (1.0f / TEMP_NTC_BETA) * logf(resistance / TEMP_NTC_R25);
    if (!isfinite(invTemperature) || invTemperature <= 0.0f)
    {
        return NAN;
    }

    return (1.0f / invTemperature) - 273.15f;
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
