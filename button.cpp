/**
 * @file button.cpp
 * @brief Implementation of Multi-Gesture Software Debounced Button Controller
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "button.h"

ButtonHandler buttonHandler(PIN_BUTTON, BUTTON_ACTIVE_LOW);

ButtonHandler::ButtonHandler(uint8_t pin, bool activeLow)
    : _pin(pin), _activeLow(activeLow), _rawState(false), _debouncedPressed(false),
      _lastRawChangeMs(0), _pressStartMs(0), _lastReleaseMs(0), _clickCount(0),
      _longPressTriggered(false), _veryLongPressTriggered(false), _factoryResetTriggered(false) {}

void ButtonHandler::begin() {
    pinMode(_pin, INPUT_PULLUP);
    _debouncedPressed = readPinRaw();
    _rawState = _debouncedPressed;
}

bool ButtonHandler::readPinRaw() {
    bool state = digitalRead(_pin);
    return _activeLow ? (state == LOW) : (state == HIGH);
}

unsigned long ButtonHandler::getPressedDuration() const {
    if (_debouncedPressed && _pressStartMs > 0) {
        return millis() - _pressStartMs;
    }
    return 0;
}

ButtonEvent ButtonHandler::update() {
    unsigned long now = millis();
    bool currentRaw = readPinRaw();
    ButtonEvent detectedEvent = BUTTON_EVENT_NONE;

    // 1. Debounce state change tracking
    if (currentRaw != _rawState) {
        _lastRawChangeMs = now;
        _rawState = currentRaw;
    }

    if ((now - _lastRawChangeMs) >= BUTTON_DEBOUNCE_MS) {
        if (currentRaw != _debouncedPressed) {
            _debouncedPressed = currentRaw;

            if (_debouncedPressed) {
                // Button Press transition
                _pressStartMs = now;
                _longPressTriggered = false;
                _veryLongPressTriggered = false;
                _factoryResetTriggered = false;
            } else {
                // Button Release transition
                unsigned long holdDuration = now - _pressStartMs;
                _lastReleaseMs = now;

                if (!_longPressTriggered && !_veryLongPressTriggered && !_factoryResetTriggered && holdDuration < BUTTON_SHORT_MAX_MS) {
                    _clickCount++;
                }
            }
        }
    }

    // 2. Active hold checks (Long / Very Long / Factory Reset while pin is held down)
    if (_debouncedPressed) {
        unsigned long holdDuration = now - _pressStartMs;

        if (holdDuration >= BUTTON_FACTORY_RESET_MS && !_factoryResetTriggered) {
            _factoryResetTriggered = true;
            _clickCount = 0;
            return BUTTON_EVENT_FACTORY_RESET;
        } else if (holdDuration >= BUTTON_VERY_LONG_MS && holdDuration < BUTTON_FACTORY_RESET_MS && !_veryLongPressTriggered) {
            _veryLongPressTriggered = true;
            _clickCount = 0;
            return BUTTON_EVENT_VERY_LONG_PRESS;
        } else if (holdDuration >= BUTTON_LONG_MIN_MS && holdDuration < BUTTON_VERY_LONG_MS && !_longPressTriggered) {
            _longPressTriggered = true;
            _clickCount = 0;
            return BUTTON_EVENT_LONG_PRESS;
        }
    }

    // 3. Multi-click evaluation after release window timeout
    if (!_debouncedPressed && _clickCount > 0) {
        if ((now - _lastReleaseMs) >= BUTTON_DOUBLE_GAP_MS) {
            if (_clickCount == 1) {
                detectedEvent = BUTTON_EVENT_SHORT_PRESS;
            } else if (_clickCount >= 2) {
                detectedEvent = BUTTON_EVENT_DOUBLE_CLICK;
            }
            _clickCount = 0;
        }
    }

    return detectedEvent;
}
