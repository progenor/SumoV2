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

private:
    void drive(int pwmLeft, int pwmRight, bool dirLeftForward, bool dirRightForward);
};

#endif // MOTORS_H
