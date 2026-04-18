#ifndef HEADING_CONTROLLER_H
#define HEADING_CONTROLLER_H

#include <Arduino.h>

class HeadingController
{
public:
    HeadingController();

    void setTunings(float proportional, float integral, float derivative);
    void setOutputLimit(float limit);
    void reset();

    float update(float errorDeg, unsigned long nowMs = 0);

private:
    float kp;
    float ki;
    float kd;

    float integral;
    float previousError;
    unsigned long lastUpdateMs;
    float outputLimit;
    float integralLimit;
};

#endif // HEADING_CONTROLLER_H