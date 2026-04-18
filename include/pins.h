#ifndef PINS_H
#define PINS_H

#include <Arduino.h>
#include "defines.h"

// SumoV2 motor driver pins (enable + pwm + direction style)
#define ENM1 11
#define ENM2 12
#define PWM1 13
#define PWM2 14
#define DIR1 3
#define DIR2 15

// Legacy DRV8243 style mapping kept for compatibility with prior experiments.
#define PWM_A1 9
#define PWM_A2 8
#define PWM_B1 20
#define PWM_B2 21
#define N_SLEEP 22

#define INT_A 27

// Buzzer Pin
#define BUZZER 2

// Current sense comes from MCP23017 inputs (not direct Pico ADC).
#define CURRENT_SENSE_A_EXP_PIN INPUT_CS_1_PIN
#define CURRENT_SENSE_B_EXP_PIN INPUT_CS_2_PIN

// Battery level monitor divider output (TPVLevel)
#define BATTERY_LEVEL_PIN 28 // GP28 ADC input

// Temperature monitor net from schematic (TM1)
#define TEMP_MONITOR_PIN 26 // GP26 ADC input (A0)

// IR nets from schematic
#define IR1_PIN 6
#define IR2_PIN 7
#define IR3_PIN 8
#define IR4_PIN 9
#define IR5_PIN 10

// Robot behavior uses a 3-sensor subset.
#define SENSOR_LEFT IR5_PIN
#define SENSOR_CENTER IR4_PIN
#define SENSOR_RIGHT IR3_PIN

// Extra two digital sensors mapped as QTR-compatible inputs.
#define QTR_LEFT IR4_PIN
#define QTR_RIGHT IR5_PIN

// Button Pin
#define BUTTON_PIN 20

const uint8_t IRPins[] = {SENSOR_LEFT, SENSOR_CENTER, SENSOR_RIGHT};
const uint8_t QTRPins[] = {QTR_LEFT, QTR_RIGHT};

void setupPins();

#endif // PINS_H