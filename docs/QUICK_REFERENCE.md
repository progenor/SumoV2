# SumoV2 Quick Reference

## Keypad Controls

Vim-style on MCP23X17:

- `h` (MCP pin 4): previous menu screen, or exit diagnostics motor test
- `l` (MCP pin 3): next menu screen, or exit diagnostics motor test
- `j` (MCP pin 5): decrease/cycle backward
- `k` (MCP pin 2): increase/cycle forward

Adjustment contexts:

- Speed screen: `j/k` changes speed preset
- Strategy screen: `j/k` changes strategy
- Diagnostics screen (`MENU_SCREEN_BATTERY`): press `k` to enter motor test
  submenu, `j/k` cycles test action, and `h/l` exits submenu.

## Menu Order (Current)

The firmware uses these indices:

- `0`: IR bars
- `1`: speed selector
- `2`: strategy selector
- `3`: diagnostics (battery + temperature + motor test submenu)
- `4`: direction + PWM
- `5`: main/logo

`MENU_SCREEN_COUNT = 6`

## Strategy Summary

- `STING`
  center attack priority; left/right turn commits for `50 ms`.
- `SPEED`
  aggressive IR attack/search.
- `RUN`
  retreat-style behavior.
- `IMU_HOLD`
  straight start phase (`550 ms`), center lock with heading PID correction,
  short recoil (`40 ms`) before full forward lock, and search/evasion/edge
  recovery state machine.

## Diagnostics Motor Test

Submenu options:

- forward
- backward
- right
- left

Execution model:

- selected action runs continuously each control cycle
- fixed test speed currently `90`
- leaving submenu restores normal strategy-driven behavior

## Battery and Temperature

Sampling:

- battery and temperature are cached and refreshed every `1000 ms`

Battery conversion used in firmware:

- `Vadc = raw / 4095 * 3.3`
- `Vbat = max(0, (Vadc + 0.12)) * 6.6`

Temperature conversion:

- 10k NTC (`B = 3950`) model from `TM1`
- invalid ranges produce `N/A`

## Buzzer Alerts

- `< 6.0V`: alarms disabled/reset
- crossing below `12.2V`: warning pattern (3 beeps)
- `< 11.3V`: continuous alarm

## Runtime Cadence

- control task: `2 ms`
- UI task: `30 ms`
- display throttle: `30 ms`
- I2C bus: `400 kHz`
- PWM: `5 kHz`, `0..255`

## Feature Flags and Debug

From `include/defines.h`:

- `ENABLE_QTR_LINE_SENSORS = 0` (default disabled)
- `DEBUG_ROBOT_BOOT = 0`
- `DEBUG_IMU_INIT = 0`
- `DEBUG_IMU_RUNTIME = 0`
- `DEBUG_I2C_ERRORS = 0`

## Useful Files

- `src/robot.cpp`: behavior + state machine + diagnostics motor test
- `src/display.cpp`: menu rendering and diagnostics screen
- `src/main.cpp`: scheduler + menu dispatch
- `lib/IMU/src/IMU.cc`: BMI323 init/read/heading integration
- `test/i2c_scanner.cpp`: standalone I2C scanner utility
