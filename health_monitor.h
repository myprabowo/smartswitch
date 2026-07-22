/**
 * @file health_monitor.h
 * @brief Self Diagnostics, Health Monitoring & RAM Ring-Buffer Logger
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef REMOTE_SWITCH_HEALTH_MONITOR_H
#define REMOTE_SWITCH_HEALTH_MONITOR_H

#include <Arduino.h>
#include <esp_system.h>
#include "config.h"
#include "ntp_manager.h"

struct LogEntry {
    String timestamp;
    LogLevel level;
    String message;
};

class HealthMonitor {
public:
    HealthMonitor();

    /**
     * @brief Performs boot self diagnostics (Flash, Heap, NVS, WiFi, HTTPS, Tuya).
     */
    void runBootDiagnostics();

    /**
     * @brief Continuous health check executed every 60 seconds.
     */
    void update();

    /**
     * @brief Adds a new log entry to RAM ring buffer and prints to Serial if permitted by log level.
     */
    void log(LogLevel level, const String& message);

    /**
     * @brief Convenience logger functions.
     */
    void logDebug(const String& msg) { log(LOG_LEVEL_DEBUG, msg); }
    void logInfo(const String& msg)  { log(LOG_LEVEL_INFO, msg); }
    void logWarn(const String& msg)  { log(LOG_LEVEL_WARN, msg); }
    void logError(const String& msg) { log(LOG_LEVEL_ERROR, msg); }

    /**
     * @brief Returns current free heap in bytes.
     */
    uint32_t getFreeHeap() const { return ESP.getFreeHeap(); }

    /**
     * @brief Returns minimum historical free heap in bytes.
     */
    uint32_t getMinFreeHeap() const { return ESP.getMinFreeHeap(); }

    /**
     * @brief Returns heap size.
     */
    uint32_t getHeapSize() const { return ESP.getHeapSize(); }

    /**
     * @brief Returns Flash size in bytes.
     */
    uint32_t getFlashSize() const { return ESP.getFlashChipSize(); }

    /**
     * @brief Returns logs as JSON string for Web Dashboard.
     */
    String getLogsJson() const;

    /**
     * @brief Clears RAM log buffer.
     */
    void clearLogs();

private:
    LogEntry _logRingBuffer[LOG_BUFFER_CAPACITY];
    uint16_t _logHead;
    uint16_t _logCount;
    unsigned long _lastHealthPulseMs;

    String logLevelToString(LogLevel lvl) const;
};

extern HealthMonitor healthMonitor;

#endif // REMOTE_SWITCH_HEALTH_MONITOR_H
