/**
 * @file led.h
 * @brief Non-blocking LED Pattern Controller for Onboard LED (GPIO8)
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef PUMP_REMOTE_LED_H
#define PUMP_REMOTE_LED_H

#include <Arduino.h>
#include "config.h"

enum LedPattern {
    LED_PATTERN_OFF = 0,
    LED_PATTERN_ON,
    LED_PATTERN_SLOW_BLINK,      // WiFi Connecting
    LED_PATTERN_SOLID_1S,        // WiFi Connected
    LED_PATTERN_TWO_SHORT_BLINKS,// Sending Tuya Command
    LED_PATTERN_ONE_LONG_BLINK,  // Command Success
    LED_PATTERN_RAPID_BLINK,     // HTTP/API Error
    LED_PATTERN_HEARTBEAT        // Provisioning Mode
};

class LedController {
public:
    LedController(uint8_t pin = PIN_LED, bool activeLow = LED_ACTIVE_LOW);

    /**
     * @brief Configures GPIO pin mode.
     */
    void begin();

    /**
     * @brief Non-blocking state update called in main loop.
     */
    void update();

    /**
     * @brief Sets active LED animation pattern.
     * @param pattern Pattern enum
     */
    void setPattern(LedPattern pattern);

    /**
     * @brief Gets current active pattern.
     */
    LedPattern getPattern() const { return _currentPattern; }

    /**
     * @brief Forces immediate physical LED state.
     * @param state true = turn LED ON, false = turn LED OFF
     */
    void setRawState(bool state);

private:
    uint8_t _pin;
    bool _activeLow;
    LedPattern _currentPattern;
    LedPattern _previousPattern;

    unsigned long _lastToggleMs;
    uint8_t _subState;
    unsigned long _oneShotEndMs;

    void applyState(bool on);
};

extern LedController ledController;

#endif // PUMP_REMOTE_LED_H
