/**
 * @file state_machine.h
 * @brief System Finite State Machine (FSM)
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef REMOTE_SWITCH_STATE_MACHINE_H
#define REMOTE_SWITCH_STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"

enum SystemState {
    STATE_BOOT = 0,
    STATE_CONNECTING_WIFI,
    STATE_CONNECTED,
    STATE_GETTING_TOKEN,
    STATE_READY,
    STATE_SENDING_COMMAND,
    STATE_ERROR,
    STATE_OTA,
    STATE_SAFE_MODE,
    STATE_FACTORY_RESET
};

class SystemFSM {
public:
    SystemFSM();

    /**
     * @brief Initializes FSM into STATE_BOOT.
     */
    void begin();

    /**
     * @brief Updates FSM logic non-blockingly inside loop().
     */
    void update();

    /**
     * @brief Transitions to target state with onExit/onEnter calls.
     * @param newState Target SystemState
     */
    void transitionTo(SystemState newState);

    /**
     * @brief Returns current active state.
     */
    SystemState getCurrentState() const { return _currentState; }

    /**
     * @brief Returns state name as string.
     */
    String getStateName(SystemState st) const;

private:
    SystemState _currentState;
    SystemState _previousState;
    unsigned long _stateEntryMs;

    void onEnter(SystemState state);
    void onUpdate(SystemState state);
    void onExit(SystemState state);
};

extern SystemFSM systemFSM;

#endif // REMOTE_SWITCH_STATE_MACHINE_H
