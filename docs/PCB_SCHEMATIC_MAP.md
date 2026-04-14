# SumoV2 PCB Schematic Map

This document captures the current whole-PCB wiring map reflected in firmware.

## Pico Core Nets

- GP2: BUZZ
- GP3: DIR1
- GP4: SDA0
- GP5: SCL0
- GP6: IR1
- GP7: IR2
- GP8: IR3
- GP9: IR4
- GP10: IR5
- GP11: ENM1C
- GP12: ENM2C
- GP13: PWM1
- GP14: PWM2
- GP15: DIR2
- GP20: BTN
- GP21: GPIO21 (general IO)
- GP26 (ADC0): TM1 (temperature monitor)
- GP27 (ADC1): INTA (MCP23017 interrupt)
- GP28 (ADC2): VLEVEL (battery divider output)

## MCP23017 Nets (I2C @ 0x22)

- GPA0 (pin index 0): BTN1
- GPA1 (pin index 1): IR6
- GPA2 (pin index 2): Keypad 3
- GPA3 (pin index 3): Keypad 4
- GPA4 (pin index 4): Keypad 1
- GPA5 (pin index 5): Keypad 2
- GPA6 (pin index 6): CS_1
- GPA7 (pin index 7): CS_2
- INTA: connected to Pico GP27

## Battery Monitor Network

- Divider: R25 = 56k, R26 = 10k
- Net: TPVLevel -> Pico GP28 (ADC2)
- Equation:
  - Vadc = Vbat \* (10 / 66)
  - Nominal: Vbat = Vadc \* 6.6
  - Firmware calibrated: Vbat = max(0, (Vadc - BATTERY_ADC_OFFSET_V)) \* 6.6

## Temperature Monitor Network

- Net: TM1 -> Pico GP26 (ADC0)
- Current firmware conversion defaults to TMP36-style transfer:
  - Temp(C) = (Vtm1 - 0.5) / 0.01
- If TM1 sensor uses a different transfer function, update:
  - TEMP_SENSOR_VOLTAGE_OFFSET
  - TEMP_SENSOR_V_PER_C
    in include/defines.h
