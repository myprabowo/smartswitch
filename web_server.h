/**
 * @file web_server.h
 * @brief Embedded Web Dashboard, Configuration Portal & REST API Server
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef PUMP_REMOTE_WEB_SERVER_H
#define PUMP_REMOTE_WEB_SERVER_H

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
     * @brief Sets current cached pump switch state.
     */
    void setCachedPumpState(bool state) { _cachedPumpState = state; }

    /**
     * @brief Returns cached pump switch state.
     */
    bool getCachedPumpState() const { return _cachedPumpState; }

private:
    WebServer _server;
    bool _cachedPumpState;

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

#endif // PUMP_REMOTE_WEB_SERVER_H
