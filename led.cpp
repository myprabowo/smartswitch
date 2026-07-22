/**
 * @file led.cpp
 * @brief Implementation of Non-Blocking LED Controller
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "led.h"

LedController ledController(PIN_LED, LED_ACTIVE_LOW);

LedController::LedController(uint8_t pin, bool activeLow)
    : _pin(pin), _activeLow(activeLow), _currentPattern(LED_PATTERN_OFF),
      _previousPattern(LED_PATTERN_OFF), _lastToggleMs(0), _subState(0), _oneShotEndMs(0) {}

void LedController::begin() {
    pinMode(_pin, OUTPUT);
    applyState(false);
}

void LedController::applyState(bool on) {
    digitalWrite(_pin, (on ^ _activeLow) ? HIGH : LOW);
}

void LedController::setRawState(bool state) {
    applyState(state);
}

void LedController::setPattern(LedPattern pattern) {
    if (_currentPattern == pattern && pattern != LED_PATTERN_SOLID_1S && pattern != LED_PATTERN_TWO_SHORT_BLINKS && pattern != LED_PATTERN_ONE_LONG_BLINK) {
        return;
    }
    _previousPattern = _currentPattern;
    _currentPattern = pattern;
    _subState = 0;
    _lastToggleMs = millis();

    if (pattern == LED_PATTERN_SOLID_1S) {
        _oneShotEndMs = millis() + 1000;
        applyState(true);
    } else if (pattern == LED_PATTERN_ONE_LONG_BLINK) {
        _oneShotEndMs = millis() + 800;
        applyState(true);
    } else if (pattern == LED_PATTERN_OFF) {
        applyState(false);
    } else if (pattern == LED_PATTERN_ON) {
        applyState(true);
    }
}

void LedController::update() {
    unsigned long now = millis();

    switch (_currentPattern) {
        case LED_PATTERN_OFF:
            applyState(false);
            break;

        case LED_PATTERN_ON:
            applyState(true);
            break;

        case LED_PATTERN_SLOW_BLINK: // 500ms ON / 500ms OFF
            if (now - _lastToggleMs >= 500) {
                _lastToggleMs = now;
                _subState = !_subState;
                applyState(_subState);
            }
            break;

        case LED_PATTERN_RAPID_BLINK: // 100ms ON / 100ms OFF
            if (now - _lastToggleMs >= 100) {
                _lastToggleMs = now;
                _subState = !_subState;
                applyState(_subState);
            }
            break;

        case LED_PATTERN_SOLID_1S: // 1000ms Solid, then revert to previous or OFF
        case LED_PATTERN_ONE_LONG_BLINK:
            if (now >= _oneShotEndMs) {
                applyState(false);
                setPattern(_previousPattern == LED_PATTERN_SLOW_BLINK ? LED_PATTERN_OFF : _previousPattern);
            }
            break;

        case LED_PATTERN_TWO_SHORT_BLINKS: // 50ms ON, 50ms OFF, 50ms ON, 50ms OFF, then revert
            if (now - _lastToggleMs >= 50) {
                _lastToggleMs = now;
                _subState++;
                if (_subState == 1) applyState(true);
                else if (_subState == 2) applyState(false);
                else if (_subState == 3) applyState(true);
                else if (_subState == 4) {
                    applyState(false);
                    setPattern(_previousPattern);
                }
            }
            break;

        case LED_PATTERN_HEARTBEAT: // 80ms ON, 100ms OFF, 80ms ON, 740ms OFF
            if (_subState == 0) { // ON 1
                applyState(true);
                if (now - _lastToggleMs >= 80) { _subState = 1; _lastToggleMs = now; }
            } else if (_subState == 1) { // OFF 1
                applyState(false);
                if (now - _lastToggleMs >= 100) { _subState = 2; _lastToggleMs = now; }
            } else if (_subState == 2) { // ON 2
                applyState(true);
                if (now - _lastToggleMs >= 80) { _subState = 3; _lastToggleMs = now; }
            } else if (_subState == 3) { // OFF 2
                applyState(false);
                if (now - _lastToggleMs >= 740) { _subState = 0; _lastToggleMs = now; }
            }
            break;
    }
}
