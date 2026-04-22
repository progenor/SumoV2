#include <Arduino.h>
#include "robot.h"
#include "button.h"
#include "menu.h"
#include "ir.h"
#include "melody.h"
#include "defines.h"

Robot robot;
ButtonManager buttonManager;

static unsigned long lastControlTaskMs = 0;
static unsigned long lastUiTaskMs = 0;

static void drawMenuScreen()
{
    if (robot.getMode() != MODE_MENU)
    {
        return;
    }

    int currentScreen = robot.getCurrentMenuScreen();
    switch (currentScreen)
    {
    case MENU_SCREEN_MAIN:
        robot.getDisplay().drawMainScreen();
        break;
    case MENU_SCREEN_SPEED:
        robot.getDisplay().drawSpeedSelectorScreen(robot.getCurrentSpeedLevel());
        break;
    case MENU_SCREEN_IR:
        robot.getDisplay().displayIR(robot.getIRValues(), IRCount);
        break;
    case MENU_SCREEN_STRATEGY:
        robot.getDisplay().drawStrategySelectorScreen(robot.getCurrentStrategy());
        break;
    case MENU_SCREEN_DIRECTION:
        robot.getDisplay().drawDirectionIndicatorScreen(robot.getCurrentDirection(), robot.getCurrentLeftMotorPWM(), robot.getCurrentRightMotorPWM());
        break;
    case MENU_SCREEN_BATTERY:
    {
        int rawBatteryAdc = robot.getBatteryRawAdc();
        float batteryAdcVoltage = robot.getBatteryAdcVoltageFromRaw(rawBatteryAdc);
        float batteryVoltage = robot.getBatteryVoltageFromRaw(rawBatteryAdc);
        robot.getDisplay().drawBatteryVoltageScreen(batteryVoltage, batteryAdcVoltage, rawBatteryAdc);
        break;
    }
    case MENU_SCREEN_TEMP:
        robot.getDisplay().drawTemperatureScreen(robot.getTemperatureC(), robot.getTemperatureVoltage());
        break;
    default:
        robot.getDisplay().drawMainScreen();
        break;
    }
}

void setup()
{
    robot.setup();
    buttonManager.setup();
}

void loop()
{
    unsigned long nowMs = millis();

    // Keep melody playback decoupled from control flow and delays.
    updateMelody();

    if ((nowMs - lastControlTaskMs) >= CONTROL_TASK_INTERVAL_MS)
    {
        buttonManager.update();
        KeypadAction action = buttonManager.getAction();
        if (action != KEYPAD_ACTION_NONE)
        {
            robot.handleKeypadAction(action);
        }

        robot.update();
        lastControlTaskMs = nowMs;
    }

    if ((nowMs - lastUiTaskMs) >= UI_TASK_INTERVAL_MS)
    {
        drawMenuScreen();
        lastUiTaskMs = nowMs;
    }
}
