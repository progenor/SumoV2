#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "pins.h"

class Motor
{
public:
    Motor();

    void setup();

    void forward(int pwm = 128);
    void backward(int pwm = 128);
    void left(int pwm = 128);
    void right(int pwm = 128);
    void stop();

    float readMotorCurrent();
    float readMotorBCurrent();

    float getFilteredMotorCurrent();
    float getFilteredMotorBCurrent();

    void updatePeaks();
    float getPeakMotorACurrent();
    float getPeakMotorBCurrent();
    float getTotalPeakCurrent();
    void resetPeaks();

private:
    static const float ALPHA_FILTER;

    float filteredCurrent_A;
    float filteredCurrent_B;
    bool isFirstRead_A;
    bool isFirstRead_B;

    float peakCurrent_A;
    float peakCurrent_B;

    void drive(int pwmLeft, int pwmRight, bool dirLeftForward, bool dirRightForward);
};

#endif // MOTORS_H
