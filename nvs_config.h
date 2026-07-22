/**
 * @file nvs_config.h
 * @brief Non-Volatile Storage (NVS) Configuration Manager using Preferences
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef PUMP_REMOTE_NVS_CONFIG_H
#define PUMP_REMOTE_NVS_CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct SystemConfig {
    String deviceName;
    String hostname;
    String otaPassword;
    String tuyaAccessId;
    String tuyaAccessSecret;
    String tuyaDeviceId;
    String tuyaEndpoint;
    String tuyaSwitchCode;
    int logLevel;
    long gmtOffsetSec;
};

class NVSConfigManager {
public:
    NVSConfigManager();

    /**
     * @brief Initializes Preferences partition and loads settings into memory.
     */
    void begin();

    /**
     * @brief Saves entire SystemConfig struct to NVS.
     * @param config New configuration struct
     * @return true if successful
     */
    bool saveConfig(const SystemConfig& config);

    /**
     * @brief Returns current loaded SystemConfig.
     */
    const SystemConfig& getConfig() const { return _config; }

    /**
     * @brief Updates individual Tuya settings in NVS.
     */
    void setTuyaCredentials(const String& accessId, const String& secret, const String& deviceId, const String& endpoint);

    /**
     * @brief Tracks crash counts across reboots for Safe Mode entry.
     * @return Current consecutive crash count
     */
    uint32_t incrementBootCount();

    /**
     * @brief Clears crash counter after clean execution.
     */
    void clearConsecutiveCrashes();

    /**
     * @brief Reads consecutive crash count.
     */
    uint32_t getConsecutiveCrashes();

    /**
     * @brief Erases all NVS configuration and resets to defaults.
     */
    void factoryReset();

    /**
     * @brief Returns total boot count.
     */
    uint32_t getBootCount() const { return _bootCount; }

    /**
     * @brief Exports configuration to JSON string (excluding sensitive raw secret if masked).
     * @param includeSecrets If true, secrets are included in raw form
     * @return JSON string
     */
    String exportJson(bool includeSecrets = false) const;

    /**
     * @brief Imports configuration from JSON string.
     * @param json Payload string
     * @return true if valid and saved
     */
    bool importJson(const String& json);

private:
    Preferences _prefs;
    SystemConfig _config;
    uint32_t _bootCount;
    uint32_t _consecutiveCrashes;

    void loadDefaults();
};

extern NVSConfigManager nvsConfigManager;

#endif // PUMP_REMOTE_NVS_CONFIG_H
