/**
 * @file wifi_manager.cpp
 * @brief Implementation of WiFi Provisioning and Connection Monitor
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "wifi_manager.h"

CustomWiFiManager customWiFiManager;

CustomWiFiManager::CustomWiFiManager()
    : _inAPMode(false), _lastReconnectAttemptMs(0), _failedConnectAttempts(0) {}

void CustomWiFiManager::setupMDNS() {
    const SystemConfig& cfg = nvsConfigManager.getConfig();
    if (MDNS.begin(cfg.hostname.c_str())) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("mDNS responder started: http://" + cfg.hostname + ".local");
    }
}

bool CustomWiFiManager::begin(bool forceAP) {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    WiFiManager wm;
    wm.setDebugOutput(false);
    wm.setConfigPortalTimeout(180); // 3 minutes timeout for portal before auto retry

    // LED Pattern during portal connection
    wm.setAPCallback([this](WiFiManager *myWiFiManager) {
        _inAPMode = true;
        ledController.setPattern(LED_PATTERN_HEARTBEAT);
        Serial.println("----------------------------------");
        Serial.println("Entering WiFi Provisioning Mode...");
        Serial.print("AP SSID: ");
        Serial.println(myWiFiManager->getConfigPortalSSID());
        Serial.print("AP IP:   ");
        Serial.println(WiFi.softAPIP().toString());
        Serial.println("----------------------------------");
    });

    bool res = false;
    if (forceAP) {
        _inAPMode = true;
        ledController.setPattern(LED_PATTERN_HEARTBEAT);
        res = wm.startConfigPortal(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);
    } else {
        ledController.setPattern(LED_PATTERN_SLOW_BLINK);
        res = wm.autoConnect(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);
    }

    if (!res) {
        Serial.println("WiFi Connection Failed or Config Portal Timeout!");
        _inAPMode = true;
        return false;
    }

    _inAPMode = false;
    ledController.setPattern(LED_PATTERN_SOLID_1S);

    Serial.println("WiFi Connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP().toString());

    setupMDNS();
    return true;
}

void CustomWiFiManager::update() {
    if (_inAPMode) return;

    unsigned long now = millis();
    if (WiFi.status() != WL_CONNECTED) {
        if (now - _lastReconnectAttemptMs >= WIFI_CHECK_INTERVAL_MS) {
            _lastReconnectAttemptMs = now;
            _failedConnectAttempts++;

            Serial.println("WiFi Connection lost! Attempting auto-reconnect...");
            ledController.setPattern(LED_PATTERN_SLOW_BLINK);
            WiFi.reconnect();

            if (_failedConnectAttempts >= 6) { // After 30 seconds of persistent failure
                Serial.println("WiFi reconnect failed repeatedly. Restarting Captive Portal...");
                begin(true);
            }
        }
    } else {
        if (_failedConnectAttempts > 0) {
            _failedConnectAttempts = 0;
            Serial.println("WiFi Reconnected successfully!");
            ledController.setPattern(LED_PATTERN_SOLID_1S);
        }
    }
}

void CustomWiFiManager::resetCredentialsAndRestart() {
    Serial.println("Wiping WiFi credentials...");
    WiFiManager wm;
    wm.resetSettings();
    delay(500);
    ESP.restart();
}

int CustomWiFiManager::getRSSI() const {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

String CustomWiFiManager::getIPAddress() const {
    if (_inAPMode) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

String CustomWiFiManager::getMACAddress() const {
    return WiFi.macAddress();
}
