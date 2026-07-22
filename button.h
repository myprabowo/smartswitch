/**
 * @file button.h
 * @brief Software Debounced Multi-Gesture Button Handler (GPIO9)
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef PUMP_REMOTE_BUTTON_H
#define PUMP_REMOTE_BUTTON_H

#include <Arduino.h>
#include "config.h"

enum ButtonEvent {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_LONG_PRESS,      // > 2s
    BUTTON_EVENT_VERY_LONG_PRESS, // > 8s
    BUTTON_EVENT_FACTORY_RESET    // > 15s
};

class ButtonHandler {
public:
    ButtonHandler(uint8_t pin = PIN_BUTTON, bool activeLow = BUTTON_ACTIVE_LOW);

    /**
     * @brief Configures GPIO input mode with internal pull-up.
     */
    void begin();

    /**
     * @brief Polled state machine to be called in loop().
     * @return Detected ButtonEvent
     */
    ButtonEvent update();

    /**
     * @brief Returns current pressed state of physical pin (after debounce).
     */
    bool isPressed() const { return _debouncedPressed; }

    /**
     * @brief Returns duration pin has been held down in milliseconds.
     */
    unsigned long getPressedDuration() const;

private:
    uint8_t _pin;
    bool _activeLow;

    bool _rawState;
    bool _debouncedPressed;
    unsigned long _lastRawChangeMs;
    unsigned long _pressStartMs;
    unsigned long _lastReleaseMs;

    uint8_t _clickCount;
    bool _longPressTriggered;
    bool _veryLongPressTriggered;
    bool _factoryResetTriggered;

    bool readPinRaw();
};

extern ButtonHandler buttonHandler;

#endif // PUMP_REMOTE_BUTTON_H
