/**
 * @file wifi_manager.h
 * @brief WiFi Provisioning, Captive Portal, and Connection Monitor
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef REMOTE_SWITCH_WIFI_MANAGER_H
#define REMOTE_SWITCH_WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFiManager.h> // Standard ESP32 WiFiManager
#include "config.h"
#include "nvs_config.h"
#include "led.h"

class CustomWiFiManager {
public:
    CustomWiFiManager();

    /**
     * @brief Initializes WiFi subsystem and attempts connection or AP captive portal.
     * @param forceAP If true, immediately launches AP captive portal
     */
    bool begin(bool forceAP = false);

    /**
     * @brief Non-blocking monitor called periodically to check connection and process DNS requests.
     */
    void update();

    /**
     * @brief Checks if WiFi is currently connected.
     */
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }

    /**
     * @brief Checks if device is currently in AP Provisioning mode.
     */
    bool isProvisioningMode() const { return _inAPMode; }

    /**
     * @brief Wipes saved WiFi credentials and reboots into AP mode.
     */
    void resetCredentialsAndRestart();

    /**
     * @brief Gets current RSSI signal strength.
     */
    int getRSSI() const;

    /**
     * @brief Gets IP address as string.
     */
    String getIPAddress() const;

    /**
     * @brief Gets MAC address as string.
     */
    String getMACAddress() const;

private:
    bool _inAPMode;
    unsigned long _lastReconnectAttemptMs;
    uint8_t _failedConnectAttempts;
    DNSServer _dnsServer;

    void setupMDNS();
};

extern CustomWiFiManager customWiFiManager;

#endif // REMOTE_SWITCH_WIFI_MANAGER_H
