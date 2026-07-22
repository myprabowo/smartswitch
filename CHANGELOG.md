# Changelog

All notable changes to the ESP32-C3 Super Mini Tuya Remote Control project are documented in this file.

## [1.0.0] - 2026-07-22

### Added
- Initial commercial production release.
- Hardware support for ESP32-C3 Super Mini (GPIO9 BOOT button, GPIO8 onboard LED).
- Full Tuya Cloud OpenAPI v1.0 client with mbedtls HMAC-SHA256 request signing and auto token refresh.
- Non-blocking software debounced multi-gesture button state machine (Short Press, Double Click, Long Press, Very Long Press, Factory Reset).
- Non-blocking LED pattern controller (Slow blink, Solid 1s, Double short blink, Long blink, Rapid blink, Heartbeat).
- Interactive WiFiManager captive portal (`PumpRemoteSetup`) with persistent NVS storage.
- Embedded single-page dark modern Web Dashboard and REST API (`GET /api/status`, `POST /api/toggle`, etc.).
- Continuous health monitoring (60s pulse), boot self-diagnostics, 100-entry RAM log ring buffer, Task Watchdog Timer, and 3-strike crash Safe Mode recovery.
- Full markdown technical documentation suite.
