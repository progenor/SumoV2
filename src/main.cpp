#include <Arduino.h>
#include "robot.h"
#include "button.h"
#include "menu.h"
#include "ir.h"

Robot robot;
ButtonManager buttonManager;

void setup()
{
  delay(2000);
  robot.setup();
  buttonManager.setup();
}

void Screen()
{
  if (robot.getMode() == MODE_MENU)
  {
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
    case MENU_SCREEN_START_ROUTINE:
      robot.getDisplay().drawStartRoutineSelectorScreen(robot.getCurrentStartRoutine());
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
}

void loop()
{
  buttonManager.update();
  KeypadAction action = buttonManager.getAction();
  if (action != KEYPAD_ACTION_NONE)
  {
    robot.handleKeypadAction(action);
  }

  // robot.testDirections();

  robot.update();

  Screen();

  delay(5);
}