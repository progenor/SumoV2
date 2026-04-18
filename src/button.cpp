#include "button.h"
#include "expander.h"
#include "defines.h"

#define BUTTON_EVENT_DEBOUNCE_MS 80

ButtonManager::ButtonManager()
    : detectedAction(KEYPAD_ACTION_NONE),
      lastEventMs(0)
{
}

void ButtonManager::setup()
{
    setupExpander();
}

void ButtonManager::update()
{
    int pin = checkExpanderInterrupt();
    if (pin < 0)
    {
        return;
    }

    unsigned long now = millis();
    if (now - lastEventMs < BUTTON_EVENT_DEBOUNCE_MS)
    {
        return;
    }

    KeypadAction action = mapExpanderPinToAction(pin);
    if (action != KEYPAD_ACTION_NONE)
    {
        detectedAction = action;
        lastEventMs = now;
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
