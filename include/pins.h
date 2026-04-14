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

// Current Sensing Pin (IPROPI)
#define IPROPI_A_PIN 26 // GP26 ADC input for motor A current sensing
#define IPROPI_B_PIN 28 // GP28 ADC input for motor B current sensing

// IR Sensor Pins
#define SENSOR_LEFT 13
#define SENSOR_CENTER 14
#define SENSOR_RIGHT 12

// QTR Sensor Pins
#define QTR_LEFT 10
#define QTR_RIGHT 11

// Button Pin
#define BUTTON_PIN 20

const uint8_t IRPins[] = {SENSOR_LEFT, SENSOR_CENTER, SENSOR_RIGHT};
const uint8_t QTRPins[] = {QTR_LEFT, QTR_RIGHT};

void setupPins();

#endif // PINS_H