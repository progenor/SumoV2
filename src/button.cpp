#include "button.h"
#include "expander.h"
#include "defines.h"

#define BUTTON_EVENT_DEBOUNCE_MS 80

ButtonManager::ButtonManager()
    : detectedGesture(GESTURE_NONE),
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

    ButtonGesture gesture = mapExpanderPinToGesture(pin);
    if (gesture != GESTURE_NONE)
    {
        detectedGesture = gesture;
        lastEventMs = now;
    }
}

ButtonGesture ButtonManager::mapExpanderPinToGesture(int pin)
{
    switch (pin)
    {
    case BTN0:
        return GESTURE_SINGLE_PRESS;
    case BTN1:
        return GESTURE_DOUBLE_PRESS;
    case Key1:
        return GESTURE_LONG_PRESS;
    default:
        return GESTURE_NONE;
    }
}

ButtonGesture ButtonManager::getGesture()
{
    ButtonGesture gesture = detectedGesture;
    detectedGesture = GESTURE_NONE;
    return gesture;
}
