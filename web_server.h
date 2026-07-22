/**
 * @file web_server.h
 * @brief Embedded Web Dashboard, Configuration Portal & REST API Server
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef REMOTE_SWITCH_WEB_SERVER_H
#define REMOTE_SWITCH_WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "nvs_config.h"
#include "tuya_api.h"
#include "wifi_manager.h"
#include "health_monitor.h"
#include "ntp_manager.h"
#include "utils.h"

class EmbeddedWebServer {
public:
    EmbeddedWebServer();

    /**
     * @brief Starts WebServer and ArduinoOTA services.
     */
    void begin();

    /**
     * @brief Polled request handler to be executed in main loop.
     */
    void update();

    /**
     * @brief Sets current cached switch state.
     */
    void setCachedSwitchState(bool state) { _cachedSwitchState = state; }

    /**
     * @brief Returns cached switch state.
     */
    bool getCachedSwitchState() const { return _cachedSwitchState; }

private:
    WebServer _server;
    bool _cachedSwitchState;

    void setupRoutes();
    void handleRoot();
    void handleApiStatus();
    void handleApiInfo();
    void handleApiLogs();
    void handleApiToggle();
    void handleApiOn();
    void handleApiOff();
    void handleApiRestart();
    void handleApiFactoryReset();
    void handleApiSaveConfig();
    void handleApiExportConfig();
    void handleApiImportConfig();
    void handleNotFound();

    String getDashboardHTML();
};

extern EmbeddedWebServer embeddedWebServer;

#endif // REMOTE_SWITCH_WEB_SERVER_H
