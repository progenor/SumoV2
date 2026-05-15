#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

enum KeypadAction
{
    KEYPAD_ACTION_NONE,
    KEYPAD_ACTION_H,
    KEYPAD_ACTION_J,
    KEYPAD_ACTION_K,
    KEYPAD_ACTION_L,
    ACTION_BTN_MENU,
    ACTION_BTN_CHANGE_SINGLE,
    ACTION_BTN_CHANGE_DOUBLE
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

    unsigned long gpa0PressTimeMs;
    int gpa0ClickState;

    unsigned long gp20PressTimeMs;
    bool gp20WasPressed;

    KeypadAction mapExpanderPinToAction(int pin);
};

#endif // BUTTON_H
