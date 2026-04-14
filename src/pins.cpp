#include "pins.h"

void setupPins()
{
    pinMode(BUZZER, OUTPUT);

    for (size_t i = 0; i < sizeof(IRPins) / sizeof(IRPins[0]); i++)
    {
        pinMode(IRPins[i], INPUT);
    }

    for (size_t i = 0; i < sizeof(QTRPins) / sizeof(QTRPins[0]); i++)
    {
        pinMode(QTRPins[i], INPUT);
    }

    pinMode(BATTERY_LEVEL_PIN, INPUT);
    pinMode(TEMP_MONITOR_PIN, INPUT);

    pinMode(INT_A, INPUT_PULLUP);
}
