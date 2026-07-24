/**
 * @file tuya_api.cpp
 * @brief Implementation of Tuya Cloud OpenAPI Client
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "tuya_api.h"
#include <time.h>

TuyaApiClient tuyaApiClient;

TuyaApiClient::TuyaApiClient()
    : _accessToken(""), _tokenExpireMs(0), _lastResponseMessage("Initialized"), _lastHttpStatusCode(0) {}

void TuyaApiClient::begin() {
    _client.setInsecure(); // Disable strict certificate chain check for lightweight Tuya HTTPS
    _client.setTimeout(HTTP_TIMEOUT_MS / 1000);
}

uint64_t TuyaApiClient::getTimestampMs() {
    time_t nowSec = time(nullptr);
    if (nowSec > 1000000000) {
        return ((uint64_t) nowSec) * 1000ULL;
    }
    // Fallback if NTP time is not synced yet
    return 1700000000000ULL + (uint64_t)millis();
}

bool TuyaApiClient::isTokenValid() const {
    if (_accessToken.length() == 0) return false;
    return millis() < _tokenExpireMs;
}

String TuyaApiClient::getTokenStatusString() const {
    if (_accessToken.length() == 0) return "No Token";
    if (millis() >= _tokenExpireMs) return "Expired";
    unsigned long remainingSec = (_tokenExpireMs - millis()) / 1000;
    return "Valid (" + String(remainingSec) + "s left)";
}

String TuyaApiClient::generateSign(const String& method, const String& urlPath, const String& body, const String& t, bool useToken) {
    const SystemConfig& cfg = nvsConfigManager.getConfig();

    // 1. Content-SHA256
    String contentHash = "";
    if (body.length() > 0) {
        contentHash = Utils::sha256Hex(body);
        contentHash.toLowerCase();
    } else {
        contentHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    }

    // 2. Canonical StringToSign
    // StringToSign = HTTPMethod + "\n" + Content-SHA256 + "\n" + Headers + "\n" + Url
    String stringToSign = method + "\n" + contentHash + "\n\n" + urlPath;

    // 3. Signature source string
    // If token present: str = client_id + access_token + t + stringToSign
    // If token omitted: str = client_id + t + stringToSign
    String str = cfg.tuyaAccessId;
    if (useToken && _accessToken.length() > 0) {
        str += _accessToken;
    }
    str += t;
    str += stringToSign;

    // 4. HMAC-SHA256 with Secret
    return Utils::hmacSha256(str, cfg.tuyaAccessSecret);
}

TuyaCommandResult TuyaApiClient::executeHttpRequest(const String& method, const String& urlPath, const String& body, String& responseOut) {
    if (WiFi.status() != WL_CONNECTED) {
        _lastResponseMessage = "WiFi Disconnected";
        return TUYA_ERR_WIFI_OFFLINE;
    }

    const SystemConfig& cfg = nvsConfigManager.getConfig();
    HTTPClient http;
    String fullUrl = cfg.tuyaEndpoint + urlPath;

    if (!http.begin(_client, fullUrl)) {
        _lastResponseMessage = "HTTP Client Init Failed";
        return TUYA_ERR_HTTP_FAIL;
    }

    http.setTimeout(HTTP_TIMEOUT_MS);

    char tBuf[32];
    snprintf(tBuf, sizeof(tBuf), "%llu", (unsigned long long)getTimestampMs());
    String tStr = String(tBuf);
    bool useToken = (urlPath.indexOf("/v1.0/token") < 0);
    String signStr = generateSign(method, urlPath, body, tStr, useToken);

    http.addHeader("client_id", cfg.tuyaAccessId);
    http.addHeader("sign", signStr);
    http.addHeader("t", tStr);
    http.addHeader("sign_method", "HMAC-SHA256");
    http.addHeader("Content-Type", "application/json");

    if (useToken && _accessToken.length() > 0) {
        http.addHeader("access_token", _accessToken);
    }

    int httpCode = 0;
    if (method == "GET") {
        httpCode = http.GET();
    } else if (method == "POST") {
        httpCode = http.POST(body);
    } else if (method == "PUT") {
        httpCode = http.PUT(body);
    }

    _lastHttpStatusCode = httpCode;

    if (httpCode <= 0) {
        _lastResponseMessage = "HTTP Request Error: " + http.errorToString(httpCode);
        http.end();
        return TUYA_ERR_HTTP_TIMEOUT;
    }

    responseOut = http.getString();
    http.end();

    if (httpCode >= 400) {
        _lastResponseMessage = "HTTP Error Code: " + String(httpCode);
        return TUYA_ERR_HTTP_FAIL;
    }

    return TUYA_SUCCESS;
}

TuyaCommandResult TuyaApiClient::refreshAccessToken(bool forceRefresh) {
    if (!forceRefresh && isTokenValid()) {
        return TUYA_SUCCESS;
    }

    Serial.println("Getting Tuya Token...");

    String urlPath = "/v1.0/token?grant_type=1";
    String responsePayload = "";
    TuyaCommandResult res = executeHttpRequest("GET", urlPath, "", responsePayload);

    if (res != TUYA_SUCCESS) {
        return res;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, responsePayload);

    if (err) {
        _lastResponseMessage = "Token JSON parse failed";
        Serial.println("Token JSON parse failed");
        return TUYA_ERR_JSON_INVALID;
    }

    bool success = doc["success"] | false;
    if (!success) {
        int code = doc["code"] | -1;
        String msg = doc["msg"] | "Unknown Error";
        _lastResponseMessage = "Tuya Token Auth Failed (Code " + String(code) + ": " + msg + ")";
        Serial.println(_lastResponseMessage);
        return TUYA_ERR_INVALID_SECRET;
    }

    _accessToken = doc["result"]["access_token"].as<String>();
    int expireSeconds = doc["result"]["expire_time"] | 7200;

    // Safety margin of 300 seconds
    _tokenExpireMs = millis() + ((unsigned long)(expireSeconds - 300) * 1000UL);
    _lastResponseMessage = "Token acquired successfully";

    Serial.println("Token acquired.");
    return TUYA_SUCCESS;
}

TuyaCommandResult TuyaApiClient::getSwitchStatus(bool& currentState) {
    TuyaCommandResult tokenRes = refreshAccessToken();
    if (tokenRes != TUYA_SUCCESS) return tokenRes;

    const SystemConfig& cfg = nvsConfigManager.getConfig();
    String urlPath = "/v1.0/devices/" + cfg.tuyaDeviceId + "/status";

    Serial.println("Reading Switch Status...");

    String responsePayload = "";
    TuyaCommandResult res = executeHttpRequest("GET", urlPath, "", responsePayload);
    if (res != TUYA_SUCCESS) return res;

    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, responsePayload);
    if (err) {
        _lastResponseMessage = "Status JSON parse failed";
        return TUYA_ERR_JSON_INVALID;
    }

    bool success = doc["success"] | false;
    if (!success) {
        int code = doc["code"] | -1;
        String msg = doc["msg"] | "";
        if (code == 1010 || code == 1004) { // Token invalid or expired
            _accessToken = "";
            return TUYA_ERR_TOKEN_EXPIRED;
        }
        _lastResponseMessage = "Tuya Query Error Code " + String(code) + (msg.length() > 0 ? ": " + msg : "");
        return TUYA_ERR_DEVICE_OFFLINE;
    }

    JsonArray statusList = doc["result"].as<JsonArray>();
    bool found = false;
    for (JsonObject obj : statusList) {
        String code = obj["code"] | "";
        if (code == cfg.tuyaSwitchCode || code == "switch_1" || code == "switch") {
            currentState = obj["value"] | false;
            found = true;
            break;
        }
    }

    if (!found && statusList.size() > 0) {
        // Fallback to first boolean value if code mismatch
        currentState = statusList[0]["value"] | false;
        found = true;
    }

    _lastResponseMessage = "Switch Status Query Success: " + String(currentState ? "ON" : "OFF");
    Serial.println("Current State: " + String(currentState ? "ON" : "OFF"));
    return TUYA_SUCCESS;
}

TuyaCommandResult TuyaApiClient::setSwitchStatus(bool turnOn) {
    TuyaCommandResult tokenRes = refreshAccessToken();
    if (tokenRes != TUYA_SUCCESS) return tokenRes;

    const SystemConfig& cfg = nvsConfigManager.getConfig();
    String urlPath = "/v1.0/devices/" + cfg.tuyaDeviceId + "/commands";

    StaticJsonDocument<256> bodyDoc;
    JsonArray commands = bodyDoc.createNestedArray("commands");
    JsonObject cmdObj = commands.createNestedObject();
    cmdObj["code"] = cfg.tuyaSwitchCode;
    cmdObj["value"] = turnOn;

    String bodyStr = "";
    serializeJson(bodyDoc, bodyStr);

    Serial.println("Sending Command...");
    Serial.println("Switch " + String(turnOn ? "ON" : "OFF"));

    String responsePayload = "";
    TuyaCommandResult res = executeHttpRequest("POST", urlPath, bodyStr, responsePayload);
    if (res != TUYA_SUCCESS) return res;

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, responsePayload);
    if (err) {
        _lastResponseMessage = "Command Response JSON parse failed";
        return TUYA_ERR_JSON_INVALID;
    }

    bool success = doc["success"] | false;
    if (!success) {
        int code = doc["code"] | -1;
        _lastResponseMessage = "Tuya Command Failed (Code " + String(code) + ")";
        Serial.println(_lastResponseMessage);
        return TUYA_ERR_DEVICE_OFFLINE;
    }

    _lastResponseMessage = "Command Sent: Switch " + String(turnOn ? "ON" : "OFF");
    Serial.println("Success.");
    return TUYA_SUCCESS;
}

TuyaCommandResult TuyaApiClient::toggleSwitch(bool& newState) {
    bool currentState = false;
    TuyaCommandResult res = getSwitchStatus(currentState);
    if (res != TUYA_SUCCESS) {
        // Default to ON if state query failed
        currentState = false;
    }
    newState = !currentState;
    return setSwitchStatus(newState);
}
