# Production Readiness & QA Testing Checklist

Use this verification checklist to validate every firmware release prior to deploying in production environments.

---

## 1. Pre-Compile Code Verification
- [ ] `config.h`: Default fallback parameters (`FIRMWARE_VERSION`, `PIN_BUTTON`, `PIN_LED`) verified.
- [ ] No hardcoded WiFi SSIDs, passwords, or personal Tuya secrets in source files.
- [ ] Arduino IDE target set to `ESP32C3 Dev Module` or `XIAO_ESP32C3`.
- [ ] Code compiles with **0 warnings** when compiler warnings set to "All".

---

## 2. WiFi & Provisioning Test Suite
- [ ] **First-Boot Provisioning**: Erase flash via `esptool.py erase_flash` -> Verify `PumpRemoteSetup` AP appears on phone.
- [ ] **Captive Portal Test**: Connect to AP -> Verify captive portal opens automatically to `192.168.4.1` -> Save credentials -> Verify connection.
- [ ] **Auto-Reconnect Test**: Power off WiFi router -> Verify LED switches to slow blink -> Power on router -> Verify auto-reconnect without reboot within 15 seconds.
- [ ] **Provisioning Wipe Test**: Press and hold BOOT button for `> 8 seconds` -> Verify WiFi credentials erased and AP re-opens.

---

## 3. Tuya Cloud OpenAPI Test Suite
- [ ] **OAuth Token Acquisition**: Inspect Serial Monitor on boot -> Verify `Getting Tuya Token... -> Token acquired.`.
- [ ] **Switch Query**: Perform Short Press -> Verify `Reading Switch Status... -> Current State: [ON/OFF]`.
- [ ] **Switch Control**: Verify physical Tuya smart plug/switch toggles state cleanly.
- [ ] **Invalid Credentials Recovery**: Enter fake secret in Web Portal -> Verify error log printed cleanly and rapid blink LED activates.

---

## 4. Hardware Button Gesture Verification (GPIO9)
- [ ] **Short Press (`<500ms`)**: Queries state and toggles target switch.
- [ ] **Double Click (`<350ms gap`)**: Forces target switch OFF regardless of state.
- [ ] **Long Press (`>2.0s`)**: Forces target switch ON regardless of state.
- [ ] **Very Long Press (`>8.0s`)**: Erases WiFi and restarts captive portal.
- [ ] **Factory Reset (`>15.0s`)**: Clears NVS partitions, wipes crash counter, and restarts device.

---

## 5. Web Server & REST API Verification
- [ ] Open `http://pumpremote.local` in browser -> Verify SPA dashboard loads dark theme.
- [ ] Send `GET /api/status` -> Verify JSON contains valid `pumpState`, `wifiRssi`, and `tokenStatus`.
- [ ] Send `POST /api/toggle` via cURL / Postman -> Verify target switch toggles and returns `200 OK`.
- [ ] Inspect Live System Logs section -> Verify 60s health pulse entries appear continuously.

---

## 6. Watchdog & Reliability Tests
- [ ] Verify Task Watchdog Timer does not trigger during normal operation (15s timeout).
- [ ] Simulate 3 consecutive crash reboots -> Verify system enters `STATE_SAFE_MODE` and presents recovery AP.
