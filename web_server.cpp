/**
 * @file web_server.cpp
 * @brief Implementation of Embedded Web Server, REST API & HTML Dashboard
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "web_server.h"
#include <ArduinoJson.h>

EmbeddedWebServer embeddedWebServer;

EmbeddedWebServer::EmbeddedWebServer() : _server(80), _cachedSwitchState(false) {}

void EmbeddedWebServer::begin() {
    setupRoutes();
    _server.begin();
    Serial.println("Embedded Web Server started on port 80");

    // Initialize Arduino OTA
    const SystemConfig& cfg = nvsConfigManager.getConfig();
    ArduinoOTA.setHostname(cfg.hostname.c_str());
    if (cfg.otaPassword.length() > 0) {
        ArduinoOTA.setPassword(cfg.otaPassword.c_str());
    }

    ArduinoOTA.onStart([]() {
        healthMonitor.logWarn("OTA Update Started...");
        ledController.setPattern(LED_PATTERN_RAPID_BLINK);
    });

    ArduinoOTA.onEnd([]() {
        healthMonitor.logInfo("OTA Update Complete. Rebooting...");
    });

    ArduinoOTA.onError([](ota_error_t error) {
        healthMonitor.logError("OTA Error [" + String(error) + "]");
    });

    ArduinoOTA.begin();
    Serial.println("ArduinoOTA Service Ready");
}

void EmbeddedWebServer::update() {
    _server.handleClient();
    ArduinoOTA.handle();
}

void EmbeddedWebServer::setupRoutes() {
    _server.on("/", HTTP_GET, std::bind(&EmbeddedWebServer::handleRoot, this));
    _server.on("/api/status", HTTP_GET, std::bind(&EmbeddedWebServer::handleApiStatus, this));
    _server.on("/api/info", HTTP_GET, std::bind(&EmbeddedWebServer::handleApiInfo, this));
    _server.on("/api/logs", HTTP_GET, std::bind(&EmbeddedWebServer::handleApiLogs, this));
    _server.on("/api/toggle", HTTP_POST, std::bind(&EmbeddedWebServer::handleApiToggle, this));
    _server.on("/api/on", HTTP_POST, std::bind(&EmbeddedWebServer::handleApiOn, this));
    _server.on("/api/off", HTTP_POST, std::bind(&EmbeddedWebServer::handleApiOff, this));
    _server.on("/api/restart", HTTP_POST, std::bind(&EmbeddedWebServer::handleApiRestart, this));
    _server.on("/api/factory-reset", HTTP_POST, std::bind(&EmbeddedWebServer::handleApiFactoryReset, this));
    _server.on("/api/config", HTTP_POST, std::bind(&EmbeddedWebServer::handleApiSaveConfig, this));
    _server.on("/api/config/export", HTTP_GET, std::bind(&EmbeddedWebServer::handleApiExportConfig, this));
    _server.on("/api/config/import", HTTP_POST, std::bind(&EmbeddedWebServer::handleApiImportConfig, this));

    _server.onNotFound(std::bind(&EmbeddedWebServer::handleNotFound, this));
}

void EmbeddedWebServer::handleRoot() {
    _server.send(200, "text/html", getDashboardHTML());
}

void EmbeddedWebServer::handleApiStatus() {
    StaticJsonDocument<768> doc;
    const SystemConfig& cfg = nvsConfigManager.getConfig();

    doc["deviceName"]     = cfg.deviceName;
    doc["hostname"]       = cfg.hostname;
    doc["firmware"]       = FIRMWARE_VERSION;
    doc["uptime"]         = Utils::formatUptime(millis());
    doc["uptimeMs"]       = millis();
    doc["switchState"]    = _cachedSwitchState ? "ON" : "OFF";
    doc["switchStateBool"]= _cachedSwitchState;
    doc["wifiRssi"]       = customWiFiManager.getRSSI();
    doc["ipAddress"]      = customWiFiManager.getIPAddress();
    doc["macAddress"]     = customWiFiManager.getMACAddress();
    doc["freeHeap"]       = ESP.getFreeHeap();
    doc["minFreeHeap"]    = ESP.getMinFreeHeap();
    doc["tokenStatus"]    = tuyaApiClient.getTokenStatusString();
    doc["lastTuyaMessage"]= tuyaApiClient.getLastResponseMessage();
    doc["currentTime"]    = ntpManager.getFormattedTime();
    doc["bootCount"]      = nvsConfigManager.getBootCount();

    String response;
    serializeJson(doc, response);
    _server.send(200, "application/json", response);
}

void EmbeddedWebServer::handleApiInfo() {
    StaticJsonDocument<512> doc;
    doc["hardware"]       = HARDWARE_MODEL;
    doc["firmware"]       = FIRMWARE_VERSION;
    doc["sdkVersion"]     = ESP.getSdkVersion();
    doc["flashSizeKB"]    = ESP.getFlashChipSize() / 1024;
    doc["heapSize"]       = ESP.getHeapSize();
    doc["cpuFreqMHz"]     = ESP.getCpuFreqMHz();

    String response;
    serializeJson(doc, response);
    _server.send(200, "application/json", response);
}

void EmbeddedWebServer::handleApiLogs() {
    _server.send(200, "application/json", healthMonitor.getLogsJson());
}

void EmbeddedWebServer::handleApiToggle() {
    bool newState = false;
    ledController.setPattern(LED_PATTERN_TWO_SHORT_BLINKS);
    TuyaCommandResult res = tuyaApiClient.toggleSwitch(newState);

    StaticJsonDocument<256> doc;
    if (res == TUYA_SUCCESS) {
        _cachedSwitchState = newState;
        doc["success"] = true;
        doc["newState"] = newState ? "ON" : "OFF";
        doc["message"] = "Switch toggled to " + String(newState ? "ON" : "OFF");
        ledController.setPattern(LED_PATTERN_ONE_LONG_BLINK);
    } else {
        doc["success"] = false;
        doc["error"] = tuyaApiClient.getLastResponseMessage();
        ledController.setPattern(LED_PATTERN_RAPID_BLINK);
    }

    String resp;
    serializeJson(doc, resp);
    _server.send(200, "application/json", resp);
}

void EmbeddedWebServer::handleApiOn() {
    ledController.setPattern(LED_PATTERN_TWO_SHORT_BLINKS);
    TuyaCommandResult res = tuyaApiClient.setSwitchStatus(true);

    StaticJsonDocument<256> doc;
    if (res == TUYA_SUCCESS) {
        _cachedSwitchState = true;
        doc["success"] = true;
        doc["newState"] = "ON";
        doc["message"] = "Switch turned ON successfully";
        ledController.setPattern(LED_PATTERN_ONE_LONG_BLINK);
    } else {
        doc["success"] = false;
        doc["error"] = tuyaApiClient.getLastResponseMessage();
        ledController.setPattern(LED_PATTERN_RAPID_BLINK);
    }

    String resp;
    serializeJson(doc, resp);
    _server.send(200, "application/json", resp);
}

void EmbeddedWebServer::handleApiOff() {
    ledController.setPattern(LED_PATTERN_TWO_SHORT_BLINKS);
    TuyaCommandResult res = tuyaApiClient.setSwitchStatus(false);

    StaticJsonDocument<256> doc;
    if (res == TUYA_SUCCESS) {
        _cachedSwitchState = false;
        doc["success"] = true;
        doc["newState"] = "OFF";
        doc["message"] = "Switch turned OFF successfully";
        ledController.setPattern(LED_PATTERN_ONE_LONG_BLINK);
    } else {
        doc["success"] = false;
        doc["error"] = tuyaApiClient.getLastResponseMessage();
        ledController.setPattern(LED_PATTERN_RAPID_BLINK);
    }

    String resp;
    serializeJson(doc, resp);
    _server.send(200, "application/json", resp);
}

void EmbeddedWebServer::handleApiRestart() {
    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Rebooting device...\"}");
    delay(500);
    ESP.restart();
}

void EmbeddedWebServer::handleApiFactoryReset() {
    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Performing factory reset...\"}");
    delay(500);
    nvsConfigManager.factoryReset();
    customWiFiManager.resetCredentialsAndRestart();
}

void EmbeddedWebServer::handleApiSaveConfig() {
    if (!_server.hasArg("plain")) {
        _server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }

    String body = _server.arg("plain");
    if (nvsConfigManager.importJson(body)) {
        _server.send(200, "application/json", "{\"success\":true,\"message\":\"Configuration saved. Rebooting...\"}");
        delay(1000);
        ESP.restart();
    } else {
        _server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
    }
}

void EmbeddedWebServer::handleApiExportConfig() {
    _server.send(200, "application/json", nvsConfigManager.exportJson(true));
}

void EmbeddedWebServer::handleApiImportConfig() {
    handleApiSaveConfig();
}

void EmbeddedWebServer::handleNotFound() {
    _server.send(404, "application/json", "{\"error\":\"Endpoint not found\"}");
}

String EmbeddedWebServer::getDashboardHTML() {
    const SystemConfig& cfg = nvsConfigManager.getConfig();
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RemoteSwitch Dashboard</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --accent-blue: #38bdf8;
            --accent-green: #22c55e;
            --accent-red: #ef4444;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --border-color: #334155;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }
        body { background: var(--bg-color); color: var(--text-main); padding: 20px; min-height: 100vh; }
        .header { display: flex; justify-content: space-between; align-items: center; padding-bottom: 20px; border-bottom: 1px solid var(--border-color); margin-bottom: 24px; }
        .header h1 { font-size: 1.5rem; color: var(--accent-blue); }
        .badge { background: #0284c7; padding: 4px 12px; borderRadius: 12px; font-size: 0.8rem; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; margin-bottom: 24px; }
        .card { background: var(--card-bg); border: 1px solid var(--border-color); border-radius: 12px; padding: 20px; box-shadow: 0 4px 6px -1px rgba(0,0,0,0.3); }
        .card h3 { font-size: 0.95rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.05em; margin-bottom: 12px; }
        .status-val { font-size: 1.8rem; font-weight: bold; margin-bottom: 8px; }
        .status-val.on { color: var(--accent-green); }
        .status-val.off { color: var(--accent-red); }
        .info-row { display: flex; justify-content: space-between; margin-bottom: 8px; font-size: 0.9rem; }
        .info-label { color: var(--text-muted); }
        .btn-group { display: flex; gap: 10px; flex-wrap: wrap; margin-top: 12px; }
        button { background: var(--accent-blue); color: #0f172a; border: none; padding: 10px 18px; font-weight: bold; border-radius: 8px; cursor: pointer; transition: 0.2s; }
        button:hover { opacity: 0.85; }
        button.btn-danger { background: var(--accent-red); color: white; }
        button.btn-success { background: var(--accent-green); color: white; }
        .log-box { background: #090d16; border: 1px solid var(--border-color); border-radius: 8px; padding: 12px; font-family: monospace; font-size: 0.82rem; max-height: 250px; overflow-y: auto; white-space: pre-wrap; }
        input, select { width: 100%; background: #090d16; border: 1px solid var(--border-color); color: white; padding: 8px 12px; border-radius: 6px; margin-bottom: 12px; }
    </style>
</head>
<body>
    <div class="header">
        <div>
            <h1 id="devName">Remote Switch Controller</h1>
            <span class="info-label" id="hostnameTag">remoteswitch.local</span>
        </div>
        <span class="badge" id="firmwareTag">v1.0.0</span>
    </div>

    <div class="grid">
        <!-- Control Card -->
        <div class="card">
            <h3>Switch Control</h3>
            <div id="switchStateDisplay" class="status-val off">UNKNOWN</div>
            <div class="btn-group">
                <button class="btn-success" onclick="controlSwitch('on')">TURN ON</button>
                <button class="btn-danger" onclick="controlSwitch('off')">TURN OFF</button>
                <button onclick="controlSwitch('toggle')">TOGGLE</button>
            </div>
        </div>

        <!-- Telemetry Card -->
        <div class="card">
            <h3>System Status</h3>
            <div class="info-row"><span class="info-label">WiFi RSSI:</span><span id="wifiRssi">- dBm</span></div>
            <div class="info-row"><span class="info-label">IP Address:</span><span id="ipAddr">-</span></div>
            <div class="info-row"><span class="info-label">Free Heap:</span><span id="freeHeap">- KB</span></div>
            <div class="info-row"><span class="info-label">Uptime:</span><span id="uptime">-</span></div>
            <div class="info-row"><span class="info-label">Tuya Token:</span><span id="tokenStatus">-</span></div>
        </div>

        <!-- Diagnostics Card -->
        <div class="card">
            <h3>Tuya Cloud Status</h3>
            <div class="info-row"><span class="info-label">Last Message:</span></div>
            <div style="font-size:0.85rem; color:var(--accent-blue); margin-bottom:12px;" id="lastTuyaMsg">-</div>
            <div class="btn-group">
                <button onclick="fetchStatus()">Refresh Status</button>
                <button class="btn-danger" onclick="rebootDevice()">Restart ESP32</button>
            </div>
        </div>
    </div>

    <!-- Configuration Section -->
    <div class="card" style="margin-bottom: 24px;">
        <h3>Configuration Portal</h3>
        <div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); margin:0;">
            <div>
                <label class="info-label">Tuya Access ID</label>
                <input type="text" id="cfgTuyaId" value=")rawliteral" + cfg.tuyaAccessId + R"rawliteral(">
            </div>
            <div>
                <label class="info-label">Tuya Access Secret</label>
                <input type="password" id="cfgTuyaSec" value=")rawliteral" + cfg.tuyaAccessSecret + R"rawliteral(">
            </div>
            <div>
                <label class="info-label">Tuya Device ID</label>
                <input type="text" id="cfgTuyaDev" value=")rawliteral" + cfg.tuyaDeviceId + R"rawliteral(">
            </div>
            <div>
                <label class="info-label">Tuya Endpoint</label>
                <input type="text" id="cfgTuyaEp" value=")rawliteral" + cfg.tuyaEndpoint + R"rawliteral(">
            </div>
        </div>
        <div class="btn-group">
            <button class="btn-success" onclick="saveConfiguration()">Save & Reboot</button>
            <button onclick="exportConfig()">Export JSON</button>
        </div>
    </div>

    <!-- Live System Logs -->
    <div class="card">
        <h3>Live System Logs (Latest 100)</h3>
        <div class="log-box" id="logContainer">Loading logs...</div>
    </div>

    <script>
        async function fetchStatus() {
            try {
                const res = await fetch('/api/status');
                const data = await res.json();
                document.getElementById('devName').innerText = data.deviceName;
                document.getElementById('hostnameTag').innerText = data.hostname + '.local';
                document.getElementById('firmwareTag').innerText = 'v' + data.firmware;
                
                const elem = document.getElementById('switchStateDisplay');
                elem.innerText = data.switchState;
                elem.className = 'status-val ' + data.switchState.toLowerCase();

                document.getElementById('wifiRssi').innerText = data.wifiRssi + ' dBm';
                document.getElementById('ipAddr').innerText = data.ipAddress;
                document.getElementById('freeHeap').innerText = Math.round(data.freeHeap / 1024) + ' KB';
                document.getElementById('uptime').innerText = data.uptime;
                document.getElementById('tokenStatus').innerText = data.tokenStatus;
                document.getElementById('lastTuyaMsg').innerText = data.lastTuyaMessage;
            } catch(e) {}
        }

        async function fetchLogs() {
            try {
                const res = await fetch('/api/logs');
                const logs = await res.json();
                let txt = '';
                logs.forEach(l => {
                    txt += `[${l.time}] [${l.level}] ${l.msg}\n`;
                });
                const container = document.getElementById('logContainer');
                container.innerText = txt;
                container.scrollTop = container.scrollHeight;
            } catch(e) {}
        }

        async function controlSwitch(action) {
            await fetch('/api/' + action, { method: 'POST' });
            setTimeout(fetchStatus, 600);
        }

        async function rebootDevice() {
            if(confirm("Are you sure you want to restart the remote controller?")) {
                await fetch('/api/restart', { method: 'POST' });
                alert("Rebooting device...");
            }
        }

        async function saveConfiguration() {
            const body = {
                tuyaAccessId: document.getElementById('cfgTuyaId').value,
                tuyaAccessSecret: document.getElementById('cfgTuyaSec').value,
                tuyaDeviceId: document.getElementById('cfgTuyaDev').value,
                tuyaEndpoint: document.getElementById('cfgTuyaEp').value
            };
            const res = await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(body)
            });
            const data = await res.json();
            alert(data.message || data.error);
        }

        function exportConfig() {
            window.location.href = '/api/config/export';
        }

        setInterval(fetchStatus, 3000);
        setInterval(fetchLogs, 4000);
        fetchStatus();
        fetchLogs();
    </script>
</body>
</html>
)rawliteral";
    return html;
}
