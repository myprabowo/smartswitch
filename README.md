# ESP32-C3 Super Mini Tuya Smart Switch Remote Control

Production-grade, commercial-quality Arduino IDE firmware for the **ESP32-C3 Super Mini** micro-controller. Functions as a dedicated, low-latency, non-blocking wireless remote control for Tuya Smart Switches (e.g. Bardi Smart Plug/Switch) connected to the Smart Life or Tuya app via the Tuya Cloud OpenAPI v1.0.

---

## Technical Highlights

- **Hardware Target**: ESP32-C3 Super Mini (USB-C powered).
- **Zero Delay / Non-Blocking**: Built using `millis()` timing, finite state machines, and a task scheduler. No blocking `delay()` calls.
- **Tuya Cloud OpenAPI v1.0 Integration**: Native HMAC-SHA256 request signing, automatic OAuth token management, expiration tracking, and retries over secure HTTPS.
- **WiFi Provisioning**: Built-in captive portal (`PumpRemoteSetup`) via `WiFiManager`. No hardcoded credentials required.
- **Embedded Web Dashboard & Portal**: Dark responsive SPA dashboard available at `http://pumpremote.local` or via IP address.
- **JSON REST API**: Full programmatic control via REST endpoints (`/api/status`, `/api/toggle`, `/api/on`, `/api/off`, `/api/restart`, `/api/config`).
- **Resilience & Safe Mode**: Task Watchdog Timer (15s timeout), 3-strike crash detector, RAM ring buffer logging (latest 100 entries), self-diagnostics on boot.
- **Multi-Gesture Button Controller**: Software-debounced multi-gesture handling on GPIO9 (BOOT button).

---

## Hardware Pinout & Gestures

### Pin Mapping (ESP32-C3 Super Mini)
| Peripheral | Pin | Logic Level | Function |
| :--- | :--- | :--- | :--- |
| **BOOT Button** | GPIO 9 | Active LOW (Pull-up) | Multi-gesture remote input |
| **Onboard LED** | GPIO 8 | Active LOW | Visual status feedback |

### Button Gesture Map (GPIO 9)
| Action | Press Duration | Function |
| :--- | :--- | :--- |
| **Short Press** | `< 500 ms` | Query switch state & Toggle (OFF → ON / ON → OFF) |
| **Double Click** | 2 clicks within `350 ms` | Force switch **OFF** |
| **Long Press** | `> 2.0 s` | Force switch **ON** |
| **Very Long Press**| `> 8.0 s` | Erase WiFi credentials & restart in AP Provisioning Mode |
| **Factory Reset** | `> 15.0 s` | Erase NVS, WiFi, logs & perform hard restart |

---

## Onboard LED Status Patterns (GPIO 8)

| State / Event | LED Behavior |
| :--- | :--- |
| **WiFi Connecting** | Slow Blink (500ms ON / 500ms OFF) |
| **WiFi Connected** | Solid ON for 1.0 second |
| **Sending Tuya Command** | Two Short Blinks (50ms ON/OFF x 2) |
| **Command Success** | One Long Blink (800ms ON) |
| **HTTP / API Error** | Rapid Blink (100ms ON / 100ms OFF) |
| **Provisioning Mode / Safe Mode**| Heartbeat Double-Pulse (80ms ON, 100ms OFF, 80ms ON, 740ms OFF) |

---

## Setup & Installation Guide

### 1. Prerequisites & Arduino IDE Setup
1. Download and install **Arduino IDE 2.x**.
2. Open `Preferences` (`Ctrl+,` or `Cmd+,`) and add the ESP32 Board Manager URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Open `Tools -> Board -> Boards Manager`, search for `esp32` by Espressif Systems, and install version `2.0.14` or later (or `3.x`).
4. Select target board: `ESP32C3 Dev Module` or `XIAO_ESP32C3`.
   - **Flash Mode**: QIO 80MHz
   - **Flash Size**: 4MB
   - **Partition Scheme**: Default 4MB with spiffs/nvs

### 2. Installing Required Libraries
Install the following libraries via `Tools -> Manage Libraries`:
- **ArduinoJson** (by Benoit Blanchon, v6.21.x or v7.x)
- **WiFiManager** (by tzapu, v2.0.16-rc.2 or later)

*(Built-in ESP32 libraries used: `WiFi`, `WebServer`, `HTTPClient`, `WiFiClientSecure`, `Preferences`, `ESPmDNS`, `ArduinoOTA`, `mbedtls`)*

### 3. Obtaining Tuya Cloud API Credentials
1. Register/Login to the [Tuya IoT Development Platform](https://iot.tuya.com/).
2. Navigate to **Cloud -> Development -> Create Cloud Project**.
3. Create a project and note down:
   - **Access ID / Client ID**
   - **Access Secret / Client Secret**
4. Under project **Devices**, link your **Smart Life** app account using **Link Tuya App Account**.
5. Select your Smart Switch/Plug device and copy the **Device ID**.
6. Identify your region's endpoint URL:
   - **Western America**: `https://openapi.tuyaus.com`
   - **Eastern America**: `https://openapi-ueaz.tuyaus.com`
   - **Europe**: `https://openapi.tuyaeu.com`
   - **China**: `https://openapi.tuyacn.com`
   - **India**: `https://openapi.tuyain.com`

---

## WiFi Provisioning & Initial Configuration

1. Power the ESP32-C3 Super Mini via USB-C.
2. On first boot, the LED will heartbeat and create an AP network named **`PumpRemoteSetup`**.
3. Connect your mobile phone or computer to **`PumpRemoteSetup`**.
4. The captive portal will open automatically (or navigate to `http://192.168.4.1`).
5. Select your home/office WiFi network and enter the password.
6. Once connected, open a browser and visit:
   - **mDNS**: `http://pumpremote.local`
   - **IP Address**: Assigned by your router (visible in Serial Monitor or router client list).
7. Scroll to the **Configuration Portal** section on the Web Dashboard and enter your `Tuya Access ID`, `Tuya Access Secret`, `Tuya Device ID`, and `Endpoint`. Click **Save & Reboot**.

---

## REST API Specification

All endpoints return JSON responses.

### `GET /api/status`
Returns real-time telemetry, pump switch state, RSSI, free memory, and Tuya token condition.

### `GET /api/info`
Returns hardware architecture, SDK version, flash size, and CPU frequency.

### `GET /api/logs`
Returns the latest 100 system log entries stored in RAM.

### `POST /api/toggle`
Toggles switch state.
```json
{ "success": true, "newState": "ON", "message": "Pump toggled to ON" }
```

### `POST /api/on`
Forces switch state to **ON**.

### `POST /api/off`
Forces switch state to **OFF**.

### `POST /api/restart`
Reboots the device.

### `POST /api/factory-reset`
Wipes NVS and restarts device in AP Provisioning Mode.

---

## Troubleshooting

| Problem | Cause | Solution |
| :--- | :--- | :--- |
| **HTTP 401 / Invalid Secret** | Incorrect Tuya Access ID or Secret | Verify credentials in Tuya Developer Console and update via Web Portal. |
| **Code 1010 / Token Expired** | Expired access token | Token auto-refreshes automatically. Check device system time/NTP. |
| **Captive Portal does not open** | Mobile phone cellular data active | Disable mobile data while connected to `PumpRemoteSetup`. Navigate to `192.168.4.1`. |
| **Device Boot Loops** | Corrupted NVS or crash threshold | Hold BOOT button for `> 15 seconds` to trigger Factory Reset. |

---

## License
Licensed under the [MIT License](LICENSE).
