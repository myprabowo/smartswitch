/**
 * @file config.h
 * @brief Master Configuration & Hardware Definitions for ESP32-C3 Super Mini Tuya Remote
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef REMOTE_SWITCH_CONFIG_H
#define REMOTE_SWITCH_CONFIG_H

#include <Arduino.h>

// ==========================================
// Firmware Metadata & Versioning
// ==========================================
#define FIRMWARE_VERSION       "1.0.0"
#define HARDWARE_MODEL         "ESP32-C3 Super Mini"
#define DEFAULT_HOSTNAME       "RemoteSwitch"
#define DEFAULT_DEVICE_NAME    "Remote Switch Controller"
#define DEFAULT_AP_SSID        "RemoteSwitchSetup"
#define DEFAULT_AP_PASSWORD    "" // Open AP for initial captive portal
#define DEFAULT_OTA_PASSWORD   "admin123"

// ==========================================
// Hardware Pinout Definitions (ESP32-C3 Super Mini)
// ==========================================
// Built-in BOOT button on ESP32-C3 Super Mini is GPIO9 (Active LOW)
#define PIN_BUTTON             9
#define BUTTON_ACTIVE_LOW      true

// Built-in LED on ESP32-C3 Super Mini is GPIO8 (Active LOW)
#define PIN_LED                8
#define LED_ACTIVE_LOW         true

// ==========================================
// Default Tuya Cloud API Configurations
// Replace defaults via Configuration Web Portal or edit here prior to flashing
// ==========================================
#ifndef TUYA_ACCESS_ID
#define TUYA_ACCESS_ID         "YOUR_TUYA_ACCESS_ID"
#endif

#ifndef TUYA_ACCESS_SECRET
#define TUYA_ACCESS_SECRET     "YOUR_TUYA_ACCESS_SECRET"
#endif

#ifndef TUYA_DEVICE_ID
#define TUYA_DEVICE_ID         "YOUR_TUYA_DEVICE_ID"
#endif

// Tuya Endpoint Data Centers:
// Western America: https://openapi.tuyaus.com
// Eastern America: https://openapi-ueaz.tuyaus.com
// Europe:          https://openapi.tuyacn.com (or https://openapi.tuyaeu.com)
// China:           https://openapi.tuyacn.com
// India:           https://openapi.tuyain.com
#ifndef TUYA_ENDPOINT
#define TUYA_ENDPOINT          "https://openapi.tuyaus.com"
#endif

// Default Switch Function Code in Tuya API (standard for smart plugs/switches: "switch_1" or "switch")
#define TUYA_SWITCH_CODE       "switch_1"

// ==========================================
// Button Timing & Gestures (Milliseconds)
// ==========================================
#define BUTTON_DEBOUNCE_MS     30
#define BUTTON_SHORT_MAX_MS    500
#define BUTTON_DOUBLE_GAP_MS   350
#define BUTTON_LONG_MIN_MS     2000
#define BUTTON_VERY_LONG_MS    8000
#define BUTTON_FACTORY_RESET_MS 15000

// ==========================================
// System Timers & Scheduler Limits
// ==========================================
#define WIFI_CHECK_INTERVAL_MS 5000
#define HEALTH_LOG_INTERVAL_MS 60000
#define NTP_SYNC_INTERVAL_MS   3600000 // 1 hour
#define HTTP_TIMEOUT_MS        10000
#define WDT_TIMEOUT_SECONDS    15
#define SAFE_MODE_CRASH_LIMIT  3

// ==========================================
// Logging System Settings
// ==========================================
#define LOG_BUFFER_CAPACITY    100
#define LOG_LINE_MAX_LEN       160

enum LogLevel {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
};

// Default log level
#define DEFAULT_LOG_LEVEL      LOG_LEVEL_INFO

// ==========================================
// NTP Server Defaults
// ==========================================
#define NTP_SERVER_1           "pool.ntp.org"
#define NTP_SERVER_2           "time.nist.gov"
#define DEFAULT_GMT_OFFSET_SEC (7 * 3600) // UTC+7 default (Jakarta/Asia)
#define DEFAULT_DAYLIGHT_OFFSET 0

#endif // REMOTE_SWITCH_CONFIG_H
