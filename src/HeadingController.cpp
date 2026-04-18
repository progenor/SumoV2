#include "HeadingController.h"

HeadingController::HeadingController()
    : kp(2.0f), ki(0.0f), kd(0.25f), integral(0.0f), previousError(0.0f),
      lastUpdateMs(0), outputLimit(80.0f), integralLimit(150.0f)
{
}

void HeadingController::setTunings(float proportional, float integralGain, float derivative)
{
    kp = proportional;
    ki = integralGain;
    kd = derivative;
}

void HeadingController::setOutputLimit(float limit)
{
    if (limit < 0.0f)
    {
        limit = 0.0f;
    }
    outputLimit = limit;
}

void HeadingController::reset()
{
    integral = 0.0f;
    previousError = 0.0f;
    lastUpdateMs = 0;
}

float HeadingController::update(float errorDeg, unsigned long nowMs)
{
    if (nowMs == 0)
    {
        nowMs = millis();
    }

    float deltaSeconds = 0.0f;
    if (lastUpdateMs != 0 && nowMs > lastUpdateMs)
    {
        deltaSeconds = static_cast<float>(nowMs - lastUpdateMs) / 1000.0f;
    }
    lastUpdateMs = nowMs;

    if (deltaSeconds > 0.0f)
    {
        integral += errorDeg * deltaSeconds;
        if (integral > integralLimit)
        {
            integral = integralLimit;
        }
        else if (integral < -integralLimit)
        {
            integral = -integralLimit;
        }
    }

    float derivative = 0.0f;
    if (deltaSeconds > 0.0f)
    {
        derivative = (errorDeg - previousError) / deltaSeconds;
    }
    previousError = errorDeg;

    float output = (kp * errorDeg) + (ki * integral) + (kd * derivative);
    if (output > outputLimit)
    {
        output = outputLimit;
    }
    else if (output < -outputLimit)
    {
        output = -outputLimit;
    }

    return output;
}