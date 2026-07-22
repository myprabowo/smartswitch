# Troubleshooting & Diagnostic Guide

This guide provides step-by-step diagnostic procedures for resolving network, hardware, Tuya API, and flash memory issues.

---

## 1. Network & WiFi Provisioning Issues

### Symptom: `RemoteSwitchSetup` AP does not appear on first boot
- **Root Cause**: WiFi credentials are already present in flash or ESP32-C3 is stuck in a boot crash loop.
- **Resolution**:
  1. Open Serial Monitor at `115200` baud.
  2. Press and hold the BOOT button (GPIO9) for **> 15 seconds** until the LED blinks rapidly to trigger a full factory reset.
  3. Alternatively, flash empty memory using `esptool.py --chip esp32c3 erase_flash`.

### Symptom: Captive Portal does not launch after connecting to `RemoteSwitchSetup`
- **Root Cause**: Smartphone cellular data is active and overriding local DNS resolution.
- **Resolution**:
  1. Turn off **Cellular / Mobile Data** on your phone while connected to `RemoteSwitchSetup`.
  2. Open a mobile browser and manually enter `http://192.168.4.1`.

---

## 2. Tuya Cloud OpenAPI Errors

### Symptom: Serial output displays `Tuya Token Auth Failed (Code 1004 / 1106: secret invalid)`
- **Root Cause**: `TUYA_ACCESS_ID` or `TUYA_ACCESS_SECRET` is incorrect or missing permissions in Tuya Cloud Console.
- **Resolution**:
  1. Log into [Tuya Cloud Console](https://iot.tuya.com/).
  2. Go to **Cloud -> Development -> Project Details**.
  3. Copy the exact **Access ID** and **Access Secret**.
  4. Ensure API Service **IoT Core** is enabled under project **API Services**.
  5. Update credentials via `http://remoteswitch.local` configuration portal.

### Symptom: Serial output displays `HTTP Error Code: 401` or `Code 1010: token expired`
- **Root Cause**: System clock unsynchronized or token expired on Tuya cloud side.
- **Resolution**:
  - The firmware automatically purges expired tokens and re-fetches a fresh token. If persistent, check router internet access and NTP sync status.

---

## 3. Hardware & Serial Monitor Troubleshooting

### Symptom: Serial output shows garbage characters on boot
- **Root Cause**: Incorrect baud rate selected.
- **Resolution**: Set Serial Monitor baud rate to **`115200`**. Note that ESP32-C3 uses native USB CDC serial (`Serial.begin(115200)`). Ensure "USB CDC On Boot" is set to "Enabled" in Arduino IDE Tools menu if using raw USB-C port.

### Symptom: Device constantly reboots every 15 seconds
- **Root Cause**: Task Watchdog Timer (WDT) triggered by blocking execution in a custom modification.
- **Resolution**: Ensure no blocking `delay()` calls or infinite loops exist inside custom tasks.
