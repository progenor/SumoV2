#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

enum KeypadAction
{
    KEYPAD_ACTION_NONE,
    KEYPAD_ACTION_H,
    KEYPAD_ACTION_J,
    KEYPAD_ACTION_K,
    KEYPAD_ACTION_L
};

class ButtonManager
{
public:
    ButtonManager();

    void setup();
    void update();

    KeypadAction getAction();

private:
    KeypadAction detectedAction;
    unsigned long lastEventMs;

    KeypadAction mapExpanderPinToAction(int pin);
};

#endif // BUTTON_H
