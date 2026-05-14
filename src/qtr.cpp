#include "qtr.h"

QTRSensors::QTRSensors()
{
    for (int i = 0; i < QTRCount; i++)
    {
        sensorValues[i] = 0;
    }
}

void QTRSensors::setup()
{

}

void QTRSensors::read()
{

    for(int i = 0; i < QTRCount; i++)
    {

        pinMode(QTRPins[i], OUTPUT);
        digitalWrite(QTRPins[i], HIGH);
        delayMicroseconds(10);
        pinMode(QTRPins[i], INPUT);
        unsigned long startTime = micros();
        while (digitalRead(QTRPins[i]) == HIGH){}

        sensorValues[i] = micros() - startTime;
    }
}

int QTRSensors::getValue(int index)
{
    if (index >= 0 && index < QTRCount)
    {
        return sensorValues[index];
    }
    return -1;
}

int *QTRSensors::getAllValues()
{
    return sensorValues;
}

void QTRSensors::printAllValues()
{
    for (int i = 0; i < QTRCount; i++)
    {
        Serial.print(sensorValues[i]);
        Serial.print(" ");
    }
    Serial.println();
}
