# SumoV2 Quick Reference

## Control Mapping (Current)

- Expander `BTN0`: cycle menu screen
- Expander `BTN1`: cycle speed (on speed screen) or strategy (on strategy screen)
- Expander `Key1`: pause/resume (long-press semantic)

## Strategy List

- `STING`: center sensor has priority for direct attack.
- `SPEED`: aggressive search and attack.
- `RUN`: reverse/retreat behavior when target is detected.
- `IMU_HOLD`: adds gyro-z based heading correction when target is not centered.

## Speed Tuning

Speed presets are in `include/menu.h` (`SPEED_PRESETS`).

- `attack`: forward/engage speed
- `search`: no-target search speed
- `turn_moderate`, `turn_gentle`: turning speeds

## How to Change Behavior Quickly

1. Change strategy constants and presets in `include/menu.h`.
2. Edit strategy logic in `src/robot.cpp`:
   - `updateBehavior_Sting()`
   - `updateBehavior_Speed()`
   - `updateBehavior_Run()`
   - `updateBehavior_IMUHold()`
3. If motor direction is inverted on hardware, adjust `Motor::drive()` direction flags in `src/motors.cpp`.
4. If pin routing changes between PCBs, update `include/pins.h` only.

## Data/Control Diagram

```mermaid
flowchart LR
    Keys[Expander keys] --> Button[ButtonManager]
    Button --> Robot[Robot state machine]
    IR[IR + QTR sensors] --> Robot
    IMU[IMU gyro/accel] --> Robot
    Robot --> Motor[Motor outputs]
    Robot --> Display[OLED screens]
```
