#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

enum ButtonGesture
{
    GESTURE_NONE,
    GESTURE_SINGLE_PRESS,
    GESTURE_DOUBLE_PRESS,
    GESTURE_LONG_PRESS
};

class ButtonManager
{
public:
    ButtonManager();

    void setup();
    void update();

    ButtonGesture getGesture();

private:
    ButtonGesture detectedGesture;
    unsigned long lastEventMs;

    ButtonGesture mapExpanderPinToGesture(int pin);
};

#endif // BUTTON_H
