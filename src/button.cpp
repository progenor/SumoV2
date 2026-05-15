#include "button.h"
#include "expander.h"
#include "defines.h"

#define BUTTON_EVENT_DEBOUNCE_MS 80
#define DOUBLE_CLICK_TIMEOUT_MS 300

ButtonManager::ButtonManager()
    : detectedAction(KEYPAD_ACTION_NONE),
      lastEventMs(0),
      gpa0PressTimeMs(0),
      gpa0ClickState(0), // 0: idle, 1: waiting for second click
      gp20PressTimeMs(0),
      gp20WasPressed(false)
{
}

void ButtonManager::setup()
{
    setupExpander();
    pinMode(REDUNDANCY_BTN_MENU_PIN, INPUT_PULLUP);
}

void ButtonManager::update()
{
    unsigned long now = millis();

    // Check GP20 (Menu Button)
    bool gp20IsPressed = (digitalRead(REDUNDANCY_BTN_MENU_PIN) == LOW);
    if (gp20IsPressed && !gp20WasPressed)
    {
        if (now - gp20PressTimeMs > BUTTON_EVENT_DEBOUNCE_MS)
        {
            detectedAction = ACTION_BTN_MENU;
            gp20PressTimeMs = now;
        }
    }
    gp20WasPressed = gp20IsPressed;

    // Handle GPA0 (EXP_PIN_0) Timeout
    if (gpa0ClickState == 1 && (now - gpa0PressTimeMs >= DOUBLE_CLICK_TIMEOUT_MS))
    {
        // Timeout reached, dispatch single click
        detectedAction = ACTION_BTN_CHANGE_SINGLE;
        gpa0ClickState = 0;
    }

    // Check expander interrupts
    int pin = checkExpanderInterrupt();
    if (pin >= 0)
    {
        if (now - lastEventMs >= BUTTON_EVENT_DEBOUNCE_MS)
        {
            if (pin == EXP_PIN_0)
            {
                if (gpa0ClickState == 0)
                {
                    // First click
                    gpa0ClickState = 1;
                    gpa0PressTimeMs = now;
                }
                else if (gpa0ClickState == 1)
                {
                    // Second click within timeout -> Double click
                    detectedAction = ACTION_BTN_CHANGE_DOUBLE;
                    gpa0ClickState = 0;
                }
            }
            else
            {
                KeypadAction action = mapExpanderPinToAction(pin);
                if (action != KEYPAD_ACTION_NONE)
                {
                    detectedAction = action;
                }
            }
            lastEventMs = now;
        }
    }
}

KeypadAction ButtonManager::mapExpanderPinToAction(int pin)
{
    switch (pin)
    {
    case KEYPAD_VIM_H_PIN:
        return KEYPAD_ACTION_H;
    case KEYPAD_VIM_J_PIN:
        return KEYPAD_ACTION_J;
    case KEYPAD_VIM_K_PIN:
        return KEYPAD_ACTION_K;
    case KEYPAD_VIM_L_PIN:
        return KEYPAD_ACTION_L;
    default:
        return KEYPAD_ACTION_NONE;
    }
}

KeypadAction ButtonManager::getAction()
{
    KeypadAction action = detectedAction;
    detectedAction = KEYPAD_ACTION_NONE;
    return action;
}
