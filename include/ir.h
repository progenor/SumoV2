#ifndef IR_H
#define IR_H

#include <Arduino.h>
#include "pins.h"

#define IRCount 3
#define DEBOUNCE_THRESHOLD 3

class IRSensors
{
public:
    IRSensors();

    void setup();
    void read();

    int getValue(int index);
    int *getAllValues();
    void printAllValues();

private:
    int sensorValues[IRCount];
    int rawValues[IRCount];
    int debounceCounter[IRCount];
};

#endif // IR_H
