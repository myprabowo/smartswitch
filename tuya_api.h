/**
 * @file tuya_api.h
 * @brief Tuya Cloud OpenAPI v1.0 HTTPS Client with HMAC-SHA256 Signatures
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef REMOTE_SWITCH_TUYA_API_H
#define REMOTE_SWITCH_TUYA_API_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "nvs_config.h"
#include "utils.h"

enum TuyaCommandResult {
    TUYA_SUCCESS = 0,
    TUYA_ERR_WIFI_OFFLINE,
    TUYA_ERR_TOKEN_EXPIRED,
    TUYA_ERR_HTTP_TIMEOUT,
    TUYA_ERR_HTTP_FAIL,
    TUYA_ERR_JSON_INVALID,
    TUYA_ERR_INVALID_SECRET,
    TUYA_ERR_DEVICE_OFFLINE
};

class TuyaApiClient {
public:
    TuyaApiClient();

    /**
     * @brief Initializes HTTPS client.
     */
    void begin();

    /**
     * @brief Obtains or refreshes Tuya OAuth access token.
     * @param forceRefresh Force token refresh even if current token is unexpired
     * @return TuyaCommandResult
     */
    TuyaCommandResult refreshAccessToken(bool forceRefresh = false);

    /**
     * @brief Queries current power state of target Tuya switch.
     * @param currentState Output reference filled with current state (true = ON, false = OFF)
     * @return TuyaCommandResult
     */
    TuyaCommandResult getSwitchStatus(bool& currentState);

    /**
     * @brief Sets target Tuya switch state.
     * @param turnOn Desired state (true = ON, false = OFF)
     * @return TuyaCommandResult
     */
    TuyaCommandResult setSwitchStatus(bool turnOn);

    /**
     * @brief Toggles target Tuya switch state based on current status.
     * @param newState Output reference set to resulting state
     * @return TuyaCommandResult
     */
    TuyaCommandResult toggleSwitch(bool& newState);

    /**
     * @brief Returns current access token.
     */
    String getAccessToken() const { return _accessToken; }

    /**
     * @brief Returns token status string for dashboard.
     */
    String getTokenStatusString() const;

    /**
     * @brief Returns last recorded HTTP response / API status message.
     */
    String getLastResponseMessage() const { return _lastResponseMessage; }

    /**
     * @brief Returns true if token is valid and unexpired.
     */
    bool isTokenValid() const;

private:
    WiFiClientSecure _client;
    String _accessToken;
    unsigned long _tokenExpireMs;
    String _lastResponseMessage;
    int _lastHttpStatusCode;

    String generateSign(const String& method, const String& urlPath, const String& body, const String& t, bool useToken);
    unsigned long getTimestampMs();
    TuyaCommandResult executeHttpRequest(const String& method, const String& urlPath, const String& body, String& responseOut);
};

extern TuyaApiClient tuyaApiClient;

#endif // REMOTE_SWITCH_TUYA_API_H
