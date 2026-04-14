#include "motors.h"

const float V_REF = 3.3f;
const float CS_OFFSET_V = 0.050f;
const float CS_SENSITIVITY_V_PER_A = 0.020f;

const float Motor::ALPHA_FILTER = 0.97f;

Motor::Motor()
    : filteredCurrent_A(0.0f),
      filteredCurrent_B(0.0f),
      isFirstRead_A(true),
      isFirstRead_B(true),
      peakCurrent_A(0.0f),
      peakCurrent_B(0.0f)
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

void Motor::forward(int pwm)
{
    drive(pwm, pwm, true, true);
}

void Motor::backward(int pwm)
{
    drive(pwm, pwm, false, false);
}

void Motor::left(int pwm)
{
    drive(pwm, pwm, false, true);
}

void Motor::right(int pwm)
{
    drive(pwm, pwm, true, false);
}

void Motor::stop()
{
    analogWrite(PWM1, 0);
    analogWrite(PWM2, 0);
}

float Motor::readMotorCurrent()
{
    int rawADC = analogRead(IPROPI_A_PIN);
    float voltage = (rawADC / 4095.0f) * V_REF;
    float current = (voltage - CS_OFFSET_V) / CS_SENSITIVITY_V_PER_A;
    return current > 0.0f ? current : 0.0f;
}

float Motor::readMotorBCurrent()
{
    int rawADC = analogRead(IPROPI_B_PIN);
    float voltage = (rawADC / 4095.0f) * V_REF;
    float current = (voltage - CS_OFFSET_V) / CS_SENSITIVITY_V_PER_A;
    return current > 0.0f ? current : 0.0f;
}

float Motor::getFilteredMotorCurrent()
{
    float rawCurrent = readMotorCurrent();

    if (isFirstRead_A)
    {
        filteredCurrent_A = rawCurrent;
        isFirstRead_A = false;
        return filteredCurrent_A;
    }

    filteredCurrent_A = (ALPHA_FILTER * rawCurrent) + ((1.0f - ALPHA_FILTER) * filteredCurrent_A);
    return filteredCurrent_A;
}

float Motor::getFilteredMotorBCurrent()
{
    float rawCurrent = readMotorBCurrent();

    if (isFirstRead_B)
    {
        filteredCurrent_B = rawCurrent;
        isFirstRead_B = false;
        return filteredCurrent_B;
    }

    filteredCurrent_B = (ALPHA_FILTER * rawCurrent) + ((1.0f - ALPHA_FILTER) * filteredCurrent_B);
    return filteredCurrent_B;
}

void Motor::updatePeaks()
{
    float currentA = getFilteredMotorCurrent();
    if (currentA > peakCurrent_A)
    {
        peakCurrent_A = currentA;
    }

    float currentB = getFilteredMotorBCurrent();
    if (currentB > peakCurrent_B)
    {
        peakCurrent_B = currentB;
    }
}

float Motor::getPeakMotorACurrent()
{
    return peakCurrent_A;
}

float Motor::getPeakMotorBCurrent()
{
    return peakCurrent_B;
}

float Motor::getTotalPeakCurrent()
{
    return peakCurrent_A + peakCurrent_B;
}

void Motor::resetPeaks()
{
    peakCurrent_A = 0.0f;
    peakCurrent_B = 0.0f;
}
