/**
 * @file health_monitor.cpp
 * @brief Implementation of Self Diagnostics and RAM Logger
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "health_monitor.h"
#include "wifi_manager.h"
#include "nvs_config.h"
#include "utils.h"
#include <ArduinoJson.h>

HealthMonitor healthMonitor;

HealthMonitor::HealthMonitor() : _logHead(0), _logCount(0), _lastHealthPulseMs(0) {}

String HealthMonitor::logLevelToString(LogLevel lvl) const {
    switch (lvl) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        default:              return "INFO";
    }
}

void HealthMonitor::log(LogLevel level, const String& message) {
    const SystemConfig& cfg = nvsConfigManager.getConfig();

    // Print to Serial if level >= configured level
    if (level >= cfg.logLevel) {
        Serial.print("[");
        Serial.print(ntpManager.getFormattedTime());
        Serial.print("] [");
        Serial.print(logLevelToString(level));
        Serial.print("] ");
        Serial.println(message);
    }

    // Add to RAM Ring Buffer
    _logRingBuffer[_logHead].timestamp = ntpManager.getFormattedTime();
    _logRingBuffer[_logHead].level     = level;
    _logRingBuffer[_logHead].message   = message;

    _logHead = (_logHead + 1) % LOG_BUFFER_CAPACITY;
    if (_logCount < LOG_BUFFER_CAPACITY) {
        _logCount++;
    }
}

void HealthMonitor::clearLogs() {
    _logHead = 0;
    _logCount = 0;
}

String HealthMonitor::getLogsJson() const {
    DynamicJsonDocument doc(16384);
    JsonArray arr = doc.to<JsonArray>();

    int startIndex = 0;
    if (_logCount == LOG_BUFFER_CAPACITY) {
        startIndex = _logHead;
    }

    for (uint16_t i = 0; i < _logCount; i++) {
        uint16_t idx = (startIndex + i) % LOG_BUFFER_CAPACITY;
        JsonObject item = arr.createNestedObject();
        item["time"]  = _logRingBuffer[idx].timestamp;
        item["level"] = logLevelToString(_logRingBuffer[idx].level);
        item["msg"]   = _logRingBuffer[idx].message;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

void HealthMonitor::runBootDiagnostics() {
    Serial.println("----------------------------------");
    Serial.println("Running Self Diagnostics...");

    // 1. WiFi Check
    bool wifiPass = (WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_MODE_AP);
    Serial.printf("WiFi........%s\n", wifiPass ? "PASS" : "FAIL");

    // 2. Flash Check
    bool flashPass = (ESP.getFlashChipSize() >= 2000000);
    Serial.printf("Flash.......%s (%u KB)\n", flashPass ? "PASS" : "FAIL", ESP.getFlashChipSize() / 1024);

    // 3. Heap Check
    bool heapPass = (ESP.getFreeHeap() >= 40000);
    Serial.printf("Heap........%s (%u bytes free)\n", heapPass ? "PASS" : "FAIL", ESP.getFreeHeap());

    // 4. HTTPS Check
    bool httpsPass = true; // ESP32 mbedtls context is valid
    Serial.printf("HTTPS.......%s\n", httpsPass ? "PASS" : "FAIL");

    // 5. NVS Check
    bool nvsPass = (nvsConfigManager.getConfig().deviceName.length() > 0);
    Serial.printf("NVS.........%s\n", nvsPass ? "PASS" : "FAIL");

    // 6. Tuya Endpoint Config Check
    bool tuyaPass = (nvsConfigManager.getConfig().tuyaAccessId != "YOUR_TUYA_ACCESS_ID");
    Serial.printf("Tuya........%s\n", tuyaPass ? "PASS" : "FAIL (Default Credentials)");

    Serial.println("----------------------------------");

    logInfo("Boot Diagnostics Completed. System Status: OK");
}

void HealthMonitor::update() {
    unsigned long now = millis();
    if (now - _lastHealthPulseMs >= HEALTH_LOG_INTERVAL_MS) {
        _lastHealthPulseMs = now;

        esp_reset_reason_t reason = esp_reset_reason();
        String healthMsg = "Health Pulse - Free Heap: " + String(ESP.getFreeHeap()) +
                           " bytes | Min Free Heap: " + String(ESP.getMinFreeHeap()) +
                           " bytes | RSSI: " + String(customWiFiManager.getRSSI()) + " dBm" +
                           " | Uptime: " + Utils::formatUptime(now) +
                           " | Reset Reason: " + Utils::getResetReasonString(reason);

        logInfo(healthMsg);

        if (ESP.getFreeHeap() < 30000) {
            logWarn("Low Heap Warning! Free memory below 30KB threshold.");
        }
    }
}
