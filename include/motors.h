#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "pins.h"

class Motor
{
public:
    Motor();

    void setup();

    void drive(int pwmLeft, int pwmRight, bool dirLeftForward, bool dirRightForward);

    void forward(int pwm = 128);
    void backward(int pwm = 128);
    void left(int pwm = 128);
    void right(int pwm = 128);
    void stop();
    void testDirections();

private:
};

#endif // MOTORS_H
