#ifndef QTR_H
#define QTR_H

#include <Arduino.h>
#include "pins.h"

#define QTRCount 2

class QTRSensors
{
public:
    QTRSensors();

    void setup();
    void read();

    int getValue(int index);
    int *getAllValues();
    void printAllValues();

private:
    int sensorValues[QTRCount];
};

#endif // QTR_H
