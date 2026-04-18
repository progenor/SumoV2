// #include <Arduino.h>
// #include "robot.h"
// #include "button.h"
// #include "menu.h"
// #include "ir.h"

// Robot robot;
// ButtonManager buttonManager;

// void setup()
// {
//   delay(2000);
//   robot.setup();
//   buttonManager.setup();
// }

// void Screen()
// {
//   if (robot.getMode() == MODE_MENU)
//   {
//     int currentScreen = robot.getCurrentMenuScreen();
//     switch (currentScreen)
//     {
//     case MENU_SCREEN_MAIN:
//       robot.getDisplay().drawMainScreen();
//       break;
//     case MENU_SCREEN_SPEED:
//       robot.getDisplay().drawSpeedSelectorScreen(robot.getCurrentSpeedLevel());
//       break;
//     case MENU_SCREEN_IR:
//       robot.getDisplay().displayIR(robot.getIRValues(), IRCount);
//       break;
//     case MENU_SCREEN_STRATEGY:
//       robot.getDisplay().drawStrategySelectorScreen(robot.getCurrentStrategy());
//       break;
//     case MENU_SCREEN_START_ROUTINE:
//       robot.getDisplay().drawStartRoutineSelectorScreen(robot.getCurrentStartRoutine());
//       break;
//     case MENU_SCREEN_DIRECTION:
//       robot.getDisplay().drawDirectionIndicatorScreen(robot.getCurrentDirection(), robot.getCurrentLeftMotorPWM(), robot.getCurrentRightMotorPWM());
//       break;
//     case MENU_SCREEN_BATTERY:
//     {
//       int rawBatteryAdc = robot.getBatteryRawAdc();
//       float batteryAdcVoltage = robot.getBatteryAdcVoltageFromRaw(rawBatteryAdc);
//       float batteryVoltage = robot.getBatteryVoltageFromRaw(rawBatteryAdc);
//       robot.getDisplay().drawBatteryVoltageScreen(batteryVoltage, batteryAdcVoltage, rawBatteryAdc);
//       break;
//     }
//     case MENU_SCREEN_TEMP:
//       robot.getDisplay().drawTemperatureScreen(robot.getTemperatureC(), robot.getTemperatureVoltage());
//       break;
//     default:
//       robot.getDisplay().drawMainScreen();
//       break;
//     }
//   }
// }

// void loop()
// {
//   buttonManager.update();
//   KeypadAction action = buttonManager.getAction();
//   if (action != KEYPAD_ACTION_NONE)
//   {
//     robot.handleKeypadAction(action);
//   }

//   // robot.testDirections();

//   robot.update();

//   Screen();

//   delay(5);
// }

#include <Arduino.h>
#include <Wire.h>

void setup()
{
  Serial.begin(115200);

  // Wait for serial connection
  unsigned long start = millis();
  while (!Serial && (millis() - start) < 3000)
  {
    delay(10);
  }

  delay(500);
  Serial.println("\n\n========================================");
  Serial.println("I2C Scanner - Scanning for devices...");
  Serial.println("========================================");

  Wire.begin();

  uint8_t deviceCount = 0;

  Serial.println("\nScanning addresses 0x00 to 0x7F:");
  Serial.println("Address | Device Name (if known)");
  Serial.println("--------|------------------------");

  for (uint8_t i = 0; i < 128; i++)
  {
    Wire.beginTransmission(i);
    uint8_t error = Wire.endTransmission();

    if (error == 0)
    {
      deviceCount++;

      // Print address in hex format
      Serial.print("0x");
      if (i < 16)
        Serial.print("0");
      Serial.print(i, HEX);
      Serial.print("    | ");

      // Identify common devices
      switch (i)
      {
      case 0x3C:
        Serial.println("SSD1306 OLED (alt addr)");
        break;
      case 0x3D:
        Serial.println("SSD1306 OLED");
        break;
      case 0x68:
        Serial.println("BMI323 IMU (addr 1) or MPU6050");
        break;
      case 0x69:
        Serial.println("BMI323 IMU (addr 2) or MPU6050 (alt)");
        break;
      case 0x39:
        Serial.println("APDS9960 (Proximity/RGB)");
        break;
      case 0x76:
        Serial.println("BMP280 or BME280 (pressure)");
        break;
      case 0x48:
        Serial.println("ADS1115 ADC");
        break;
      case 0x20:
        Serial.println("MCP23017 I/O Expander");
        break;
      default:
        Serial.println("Unknown device");
        break;
      }
    }
  }

  Serial.println("========================================");
  Serial.print("Total devices found: ");
  Serial.println(deviceCount);
  Serial.println("========================================\n");
}

void loop()
{
  Serial.println("Scan complete. Running continuous probe on BMI323 addresses...\n");

  // Continuous monitoring of BMI323 addresses
  for (uint8_t addr = 0; addr < 2; addr++)
  {
    uint8_t bmi323_addr = (addr == 0) ? 0x68 : 0x69;

    Wire.beginTransmission(bmi323_addr);
    Wire.write(0x00);                            // Try reading from chip ID register
    uint8_t error = Wire.endTransmission(false); // Repeated start

    Serial.print("Probe 0x");
    if (bmi323_addr < 16)
      Serial.print("0");
    Serial.print(bmi323_addr, HEX);
    Serial.print(": ");

    if (error == 0)
    {
      // Try to read 2 bytes
      Wire.requestFrom(bmi323_addr, (uint8_t)2);
      if (Wire.available() >= 2)
      {
        uint8_t byte1 = Wire.read();
        uint8_t byte2 = Wire.read();
        Serial.print("✓ ACK - Read 0x");
        if (byte1 < 16)
          Serial.print("0");
        Serial.print(byte1, HEX);
        Serial.print(" 0x");
        if (byte2 < 16)
          Serial.print("0");
        Serial.println(byte2, HEX);
      }
      else
      {
        Serial.println("✓ ACK - No data returned");
      }
    }
    else if (error == 1)
    {
      Serial.println("✗ NACK on address (error 1)");
    }
    else if (error == 4)
    {
      Serial.println("✗ NACK on data (error 4)");
    }
    else
    {
      Serial.print("✗ Error ");
      Serial.println(error);
    }
  }

  Serial.println("");
  delay(2000);
}
