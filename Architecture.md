# Firmware Software Architecture

This document describes the design principles, internal module layout, finite state machine, memory management, and security model of the ESP32-C3 Super Mini Tuya Switch Remote Control firmware.

---

## Architectural Objectives

1. **Non-Blocking Core**: Avoid `delay()` across all execution paths. All delays, animations, state transitions, HTTP calls, and network monitors use non-blocking `millis()` timing.
2. **Modular Encapsulation**: Clean separation between peripheral hardware drivers, cryptographic helpers, cloud communication APIs, web server endpoints, and application logic.
3. **State-Driven Operation**: System behavior is explicitly driven by a Finite State Machine (FSM) with strict `onEnter()`, `update()`, and `onExit()` lifecycle hooks.
4. **Crash Resiliency & Diagnostics**: Continuous monitoring of heap, stack, WiFi link, watchdog, and consecutive crash history to guarantee high availability in daily real-world deployment.

---

## System Component Diagram

```
+-----------------------------------------------------------------------------------+
|                                     main.ino                                      |
|                                                                                   |
|    setup() : Hardware Init -> NVS Load -> Self-Diag -> WiFi Init -> Web Server    |
|    loop()  : Non-Blocking Scheduler Execution Loop                                |
+----------------------------------------+------------------------------------------+
                                         |
               +-------------------------+-------------------------+
               |                                                   |
+--------------v---------------+                         +---------v----------------+
|       Task Scheduler         |                         |   System State Machine   |
|   - Task Watchdog (15s)      |                         |   - FSM States & Transitions|
|   - Periodic Task Queue      |                         |   - State Handlers       |
+--------------+---------------+                         +---------+----------------+
               |                                                   |
   +-----------+-----------+---------------+                       |
   |                       |               |                       |
+--v------------+   +------v-----+   +-----v--------+              |
| ButtonHandler |   | LedControl |   | HealthMonitor|              |
| (GPIO 9)      |   | (GPIO 8)   |   | & RAM Logger |              |
+---------------+   +------------+   +--------------+              |
                                                                   |
   +---------------------------------------------------------------+
   |
+--v------------------+       +-------------------+       +-----------------------+
|  CustomWiFiManager  |       |   TuyaApiClient   |       |   EmbeddedWebServer   |
|  - WiFiManager AP   |       |   - HMAC-SHA256   |       |   - Dashboard SPA     |
|  - mDNS             |       |   - Token Cache   |       |   - REST API          |
+---------------------+       +-------------------+       +-----------------------+
```

---

## Finite State Machine (FSM)

The system operates across 10 distinct state machine states:

```
[BOOT] ---> [CONNECTING_WIFI] ---> [CONNECTED] ---> [GETTING_TOKEN] ---> [READY]
  |               |                                                         |
  | (Crash >= 3)  | (Persistent Loss)                                       | (Button Press / REST)
  v               v                                                         v
[SAFE_MODE] <---+                                                   [SENDING_COMMAND]
  |                                                                         |
  | (Manual Reset)                                                          v
  +-------------------------------------------------------------------> [ERROR]
```

### State Definitions
1. **`STATE_BOOT`**: Power-on initialization, hardware setup, NVS reading, self-diagnostics.
2. **`STATE_CONNECTING_WIFI`**: Connecting to saved AP or running captive portal server (`RemoteSwitchSetup`).
3. **`STATE_CONNECTED`**: WiFi link established, mDNS registered.
4. **`STATE_GETTING_TOKEN`**: Requesting or renewing Tuya OAuth2.0 access token via HTTPS.
5. **`STATE_READY`**: Idle operational state listening for physical button events and web API calls.
6. **`STATE_SENDING_COMMAND`**: Transmitting switch control payload to Tuya Cloud OpenAPI.
7. **`STATE_ERROR`**: Non-fatal transient error state (rapid LED blink, error logging).
8. **`STATE_OTA`**: Wireless firmware flashing active via ArduinoOTA.
9. **`STATE_SAFE_MODE`**: Activated if 3 consecutive crashes occur; opens recovery AP.
10. **`STATE_FACTORY_RESET`**: Wipes all NVS partitions, clears credentials, and restarts.

---

## Tuya Cloud OpenAPI Security & Signing Architecture

Tuya OpenAPI v1.0 requires request signing using `HMAC-SHA256` to ensure request authenticity and guard against replay attacks.

### Signature Digest Equation
```
StringToSign = HTTPMethod + "\n" + Content-SHA256 + "\n" + Headers + "\n" + UrlPath
sign = UPPERCASE( HMAC-SHA256( ClientID + AccessToken + Timestamp + StringToSign, ClientSecret ) )
```

### Cryptographic Pipeline Implementation
1. **Hash Engine**: Hardware-accelerated `mbedtls/md.h` included in ESP32 ROM.
2. **Token Caching**: Access tokens are cached in RAM with a 300-second safety margin prior to expiration.
3. **Retry Circuit Breaker**: Up to 3 request retries before flagging an error and notifying health monitor.

---

## Memory Management & Heap Protection

- **No String Allocation Churn in Loop**: Static buffers and `reserve()` are used for String operations.
- **Fixed Capacity RAM Logger**: Fixed ring buffer containing 100 structs to cap memory consumption under 16KB.
- **Static JSON Allocation**: ArduinoJson documents use explicit capacity budgets (`StaticJsonDocument<512>` / `DynamicJsonDocument<16384>`).
- **Low-Memory Warning Threshold**: Health monitor emits log warnings if free heap drops below 30KB.
