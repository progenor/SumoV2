#include "motors.h"

Motor::Motor()
{
}

void Motor::setup()
{
    pinMode(ENM1, OUTPUT);
    pinMode(ENM2, OUTPUT);
    pinMode(PWM1, OUTPUT);
    pinMode(PWM2, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(DIR2, OUTPUT);

    analogReadResolution(12);
    analogWriteRange(255);
    analogWriteFreq(5000);

    digitalWrite(ENM1, HIGH);
    digitalWrite(ENM2, HIGH);
    stop();
}

void Motor::drive(int pwmLeft, int pwmRight, bool dirLeftForward, bool dirRightForward)
{
    digitalWrite(DIR1, dirLeftForward ? HIGH : LOW);
    digitalWrite(DIR2, dirRightForward ? HIGH : LOW);

    analogWrite(PWM1, constrain(pwmLeft, 0, 255));
    analogWrite(PWM2, constrain(pwmRight, 0, 255));
}

void Motor::backward(int pwm)
{
    drive(pwm, pwm, true, false);
}

void Motor::forward(int pwm)
{
    drive(pwm, pwm, false, true);
}

void Motor::right(int pwm)
{
    drive(pwm, pwm, true, true);
}

void Motor::left(int pwm)
{
    drive(pwm, pwm, false, false);
}

void Motor::stop()
{
    analogWrite(PWM1, 0);
    analogWrite(PWM2, 0);
}

void Motor::testDirections()
{
    forward(128);
    delay(1000);
    backward(128);
    delay(1000);
    left(128);
    delay(1000);
    right(128);
    delay(1000);
    stop();
}