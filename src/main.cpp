#include <Arduino.h>
#include "robot.h"
#include "button.h"
#include "menu.h"
#include "ir.h"

Robot robot;
ButtonManager buttonManager;

void setup()
{
  robot.setup();
  buttonManager.setup();
}

void loop()
{
  buttonManager.update();
  KeypadAction action = buttonManager.getAction();
  if (action != KEYPAD_ACTION_NONE)
  {
    robot.handleKeypadAction(action);
  }

  robot.update();

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
    case MENU_SCREEN_CURRENT:
    {
      char motorA[12];
      char motorB[12];
      snprintf(motorA, sizeof(motorA), "%.2f", robot.getMotor().getFilteredMotorCurrent());
      snprintf(motorB, sizeof(motorB), "%.2f", robot.getMotor().getFilteredMotorBCurrent());
      robot.getDisplay().drawCurentReading(motorA, motorB);
      break;
    }
    case MENU_SCREEN_PEAK_CURRENT:
    {
      char peakA[12];
      char peakB[12];
      char peakTotal[12];
      snprintf(peakA, sizeof(peakA), "%.2f", robot.getMotor().getPeakMotorACurrent());
      snprintf(peakB, sizeof(peakB), "%.2f", robot.getMotor().getPeakMotorBCurrent());
      snprintf(peakTotal, sizeof(peakTotal), "%.2f", robot.getMotor().getTotalPeakCurrent());
      robot.getDisplay().drawPEAK_Current(peakA, peakB, peakTotal);
      break;
    }
    case MENU_SCREEN_IR:
      robot.getDisplay().displayIR(robot.getIRValues(), IRCount);
      break;
    case MENU_SCREEN_STRATEGY:
      robot.getDisplay().drawStrategySelectorScreen(robot.getCurrentStrategy());
      break;
    case MENU_SCREEN_DIRECTION:
      robot.getDisplay().drawDirectionIndicatorScreen(robot.getCurrentDirection());
      break;
    case MENU_SCREEN_BATTERY:
      robot.getDisplay().drawBatteryVoltageScreen(robot.getBatteryVoltage());
      break;
    case MENU_SCREEN_TEMP:
      robot.getDisplay().drawTemperatureScreen(robot.getTemperatureC(), robot.getTemperatureVoltage());
      break;
    default:
      robot.getDisplay().drawMainScreen();
      break;
    }
  }

  delay(5);
}