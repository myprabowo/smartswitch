/**
 * @file ntp_manager.cpp
 * @brief Implementation of NTP Manager
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "ntp_manager.h"

NTPManager ntpManager;

NTPManager::NTPManager() : _synced(false), _lastCheckMs(0) {}

void NTPManager::begin() {
    const SystemConfig& cfg = nvsConfigManager.getConfig();
    configTime(cfg.gmtOffsetSec, DEFAULT_DAYLIGHT_OFFSET, NTP_SERVER_1, NTP_SERVER_2);
}

void NTPManager::update() {
    unsigned long now = millis();
    if (now - _lastCheckMs >= 10000) { // Check every 10 seconds until synced
        _lastCheckMs = now;
        time_t rawtime;
        time(&rawtime);
        if (rawtime > 1000000000) {
            if (!_synced) {
                _synced = true;
                Serial.println("NTP Time Synchronized: " + getFormattedTime());
            }
        }
    }
}

String NTPManager::getFormattedTime() const {
    time_t rawtime;
    time(&rawtime);
    if (rawtime < 1000000000) {
        return "NTP Unsynced (" + String(millis() / 1000) + "s uptime)";
    }
    struct tm timeinfo;
    localtime_r(&rawtime, &timeinfo);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buf);
}
