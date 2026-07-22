/**
 * @file state_machine.cpp
 * @brief Implementation of System Finite State Machine
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "state_machine.h"
#include "led.h"
#include "wifi_manager.h"
#include "tuya_api.h"
#include "health_monitor.h"
#include "nvs_config.h"
#include "utils.h"

SystemFSM systemFSM;

SystemFSM::SystemFSM()
    : _currentState(STATE_BOOT), _previousState(STATE_BOOT), _stateEntryMs(0) {}

String SystemFSM::getStateName(SystemState st) const {
    switch (st) {
        case STATE_BOOT:             return "BOOT";
        case STATE_CONNECTING_WIFI:  return "CONNECTING_WIFI";
        case STATE_CONNECTED:        return "CONNECTED";
        case STATE_GETTING_TOKEN:    return "GETTING_TOKEN";
        case STATE_READY:            return "READY";
        case STATE_SENDING_COMMAND:  return "SENDING_COMMAND";
        case STATE_ERROR:            return "ERROR";
        case STATE_OTA:              return "OTA";
        case STATE_SAFE_MODE:        return "SAFE_MODE";
        case STATE_FACTORY_RESET:    return "FACTORY_RESET";
        default:                     return "UNKNOWN";
    }
}

void SystemFSM::begin() {
    onEnter(STATE_BOOT);
}

void SystemFSM::transitionTo(SystemState newState) {
    if (_currentState == newState) return;

    onExit(_currentState);
    _previousState = _currentState;
    _currentState = newState;
    _stateEntryMs = millis();
    onEnter(_currentState);
}

void SystemFSM::onEnter(SystemState state) {
    healthMonitor.logInfo("FSM Transition -> " + getStateName(state));

    switch (state) {
        case STATE_BOOT:
            ledController.setPattern(LED_PATTERN_ON);
            break;

        case STATE_CONNECTING_WIFI:
            ledController.setPattern(LED_PATTERN_SLOW_BLINK);
            break;

        case STATE_CONNECTED:
            ledController.setPattern(LED_PATTERN_SOLID_1S);
            break;

        case STATE_GETTING_TOKEN:
            ledController.setPattern(LED_PATTERN_SLOW_BLINK);
            break;

        case STATE_READY:
            ledController.setPattern(LED_PATTERN_OFF);
            nvsConfigManager.clearConsecutiveCrashes();
            break;

        case STATE_SENDING_COMMAND:
            ledController.setPattern(LED_PATTERN_TWO_SHORT_BLINKS);
            break;

        case STATE_ERROR:
            ledController.setPattern(LED_PATTERN_RAPID_BLINK);
            break;

        case STATE_OTA:
            ledController.setPattern(LED_PATTERN_RAPID_BLINK);
            break;

        case STATE_SAFE_MODE:
            ledController.setPattern(LED_PATTERN_HEARTBEAT);
            healthMonitor.logWarn("ENTERING SAFE MODE! Crash threshold reached.");
            break;

        case STATE_FACTORY_RESET:
            ledController.setPattern(LED_PATTERN_RAPID_BLINK);
            healthMonitor.logWarn("Performing Factory Reset...");
            break;
    }
}

void SystemFSM::onUpdate(SystemState state) {
    switch (state) {
        case STATE_BOOT:
            // Boot setup completed in main.ino setup(), transition to WiFi
            if (nvsConfigManager.getConsecutiveCrashes() >= SAFE_MODE_CRASH_LIMIT) {
                transitionTo(STATE_SAFE_MODE);
            } else {
                transitionTo(STATE_CONNECTING_WIFI);
            }
            break;

        case STATE_CONNECTING_WIFI:
            if (customWiFiManager.isConnected()) {
                transitionTo(STATE_CONNECTED);
            }
            break;

        case STATE_CONNECTED:
            if (millis() - _stateEntryMs >= 1000) { // After 1s solid LED
                transitionTo(STATE_GETTING_TOKEN);
            }
            break;

        case STATE_GETTING_TOKEN:
            if (tuyaApiClient.refreshAccessToken() == TUYA_SUCCESS) {
                transitionTo(STATE_READY);
            } else {
                // Non-blocking retry handled in scheduler/tuya client
                if (millis() - _stateEntryMs > 10000) {
                    transitionTo(STATE_READY); // Move to ready anyway to allow local web/API controls
                }
            }
            break;

        case STATE_READY:
            if (!customWiFiManager.isConnected() && !customWiFiManager.isProvisioningMode()) {
                transitionTo(STATE_CONNECTING_WIFI);
            }
            break;

        case STATE_SENDING_COMMAND:
            // Handled during button/API command execution
            if (millis() - _stateEntryMs > 3000) {
                transitionTo(STATE_READY);
            }
            break;

        case STATE_ERROR:
            if (millis() - _stateEntryMs > 5000) {
                transitionTo(STATE_READY);
            }
            break;

        case STATE_SAFE_MODE:
            // In safe mode, AP remains active for recovery
            break;

        case STATE_FACTORY_RESET:
            nvsConfigManager.factoryReset();
            customWiFiManager.resetCredentialsAndRestart();
            break;

        case STATE_OTA:
            break;
    }
}

void SystemFSM::onExit(SystemState state) {
    // Cleanup if necessary
}

void SystemFSM::update() {
    onUpdate(_currentState);
}
