#include "ir.h"

IRSensors::IRSensors()
{
    for (int i = 0; i < IRCount; i++)
    {
        sensorValues[i] = 0;
        rawValues[i] = 0;
        debounceCounter[i] = 0;
    }
}

void IRSensors::setup()
{
}

void IRSensors::read()
{
    for (int i = 0; i < IRCount; i++)
    {
        int rawRead = digitalRead(IRPins[i]);
        rawValues[i] = rawRead;

        if (rawRead == sensorValues[i])
        {
            debounceCounter[i] = 0;
        }
        else
        {
            debounceCounter[i]++;
            if (debounceCounter[i] >= DEBOUNCE_THRESHOLD)
            {
                sensorValues[i] = rawRead;
                debounceCounter[i] = 0;
            }
        }
    }
}

int IRSensors::getValue(int index)
{
    if (index >= 0 && index < IRCount)
    {
        return sensorValues[index];
    }
    return -1;
}

int *IRSensors::getAllValues()
{
    return sensorValues;
}

void IRSensors::printAllValues()
{
    for (int i = 0; i < IRCount; i++)
    {
        Serial.print(sensorValues[i]);
        Serial.print(" ");
    }
    Serial.println();
}
