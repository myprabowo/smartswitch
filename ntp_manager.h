/**
 * @file ntp_manager.h
 * @brief NTP Time Sync & Formatting Manager
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef REMOTE_SWITCH_NTP_MANAGER_H
#define REMOTE_SWITCH_NTP_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include "config.h"
#include "nvs_config.h"

class NTPManager {
public:
    NTPManager();

    /**
     * @brief Configures NTP time sync settings.
     */
    void begin();

    /**
     * @brief Periodic update to check sync status.
     */
    void update();

    /**
     * @brief Checks if time has been successfully synchronized with NTP.
     */
    bool isSynced() const { return _synced; }

    /**
     * @brief Returns current formatted ISO timestamp: YYYY-MM-DD HH:MM:SS.
     */
    String getFormattedTime() const;

private:
    bool _synced;
    unsigned long _lastCheckMs;
};

extern NTPManager ntpManager;

#endif // REMOTE_SWITCH_NTP_MANAGER_H
