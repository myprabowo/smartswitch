# REST API Reference Manual

The ESP32-C3 Super Mini Tuya Remote Controller provides a complete JSON REST API on port `80`.

**Base URL**: `http://pumpremote.local` or `http://<device-ip>`

---

## Endpoint Overview

| Method | Endpoint | Description | Content-Type |
| :--- | :--- | :--- | :--- |
| `GET` | `/api/status` | Returns system status & telemetry | `application/json` |
| `GET` | `/api/info` | Returns hardware & firmware details | `application/json` |
| `GET` | `/api/logs` | Returns latest 100 system logs | `application/json` |
| `POST` | `/api/toggle` | Toggles Tuya switch state | `application/json` |
| `POST` | `/api/on` | Forces Tuya switch state **ON** | `application/json` |
| `POST` | `/api/off` | Forces Tuya switch state **OFF** | `application/json` |
| `POST` | `/api/restart` | Reboots the ESP32 microcontroller | `application/json` |
| `POST` | `/api/factory-reset` | Erases NVS configuration and reboots | `application/json` |
| `POST` | `/api/config` | Updates NVS system configuration | `application/json` |
| `GET` | `/api/config/export` | Downloads system configuration JSON | `application/json` |

---

## Detailed Endpoint Documentation

### 1. `GET /api/status`
Returns real-time system telemetry and pump switch status.

**Sample Response**:
```json
{
  "deviceName": "Pump Controller Remote",
  "hostname": "PumpRemote",
  "firmware": "1.0.0",
  "uptime": "00h 42m 15s",
  "uptimeMs": 2535000,
  "pumpState": "OFF",
  "pumpStateBool": false,
  "wifiRssi": -58,
  "ipAddress": "192.168.1.105",
  "macAddress": "74:4D:BD:12:34:56",
  "freeHeap": 184520,
  "minFreeHeap": 172100,
  "tokenStatus": "Valid (6840s left)",
  "lastTuyaMessage": "Switch Status Query Success: OFF",
  "currentTime": "2026-07-22 20:15:30",
  "bootCount": 12
}
```

---

### 2. `POST /api/toggle`
Toggles switch state using current status query.

**Sample Response**:
```json
{
  "success": true,
  "newState": "ON",
  "message": "Pump toggled to ON"
}
```

---

### 3. `POST /api/on`
Forces switch state to ON.

**Sample Response**:
```json
{
  "success": true,
  "newState": "ON",
  "message": "Pump turned ON successfully"
}
```

---

### 4. `POST /api/off`
Forces switch state to OFF.

**Sample Response**:
```json
{
  "success": true,
  "newState": "OFF",
  "message": "Pump turned OFF successfully"
}
```

---

### 5. `POST /api/config`
Updates NVS system parameters and initiates a reboot.

**Sample Request Body**:
```json
{
  "tuyaAccessId": "tuya_access_id_here",
  "tuyaAccessSecret": "tuya_secret_here",
  "tuyaDeviceId": "tuya_device_id_here",
  "tuyaEndpoint": "https://openapi.tuyaus.com"
}
```

**Sample Response**:
```json
{
  "success": true,
  "message": "Configuration saved. Rebooting..."
}
```
