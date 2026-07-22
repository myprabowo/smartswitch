# ESP32-C3 Memory & Flash Usage Estimate

This document provides a technical breakdown of RAM (SRAM) and Flash memory consumption for the ESP32-C3 Super Mini.

---

## Hardware Memory Specifications (ESP32-C3)
- **Internal SRAM**: 400 KB
- **Flash Memory**: 4 MB (32 Mbit)

---

## SRAM (RAM) Allocation Breakdown

| Subsystem / Data Structure | Memory Allocation | Purpose |
| :--- | :--- | :--- |
| **System Free Heap Baseline** | ~240.0 KB | Available dynamic heap for HTTPS SSL buffers & Web requests |
| **mbedtls TLS Context (WiFiClientSecure)** | ~35.0 KB | Transient buffer during Tuya HTTPS calls |
| **RAM Logger Ring Buffer** | ~16.0 KB | Fixed buffer holding 100 log structs (160 bytes each) |
| **WiFi / TCP/IP Stack Stack** | ~65.0 KB | ESP-IDF LwIP network buffers & WiFi driver internals |
| **NVS / Configuration Cache** | ~2.0 KB | SystemConfig struct & Preferences handle |
| **FSM & Scheduler Handles** | ~1.5 KB | Task array (8 slots) and state tracking variables |
| **Static & Global Variables** | ~8.0 KB | Class instances (`ledController`, `buttonHandler`, etc.) |

---

## Flash Memory Allocation Breakdown (4MB Partition Scheme)

| Partition | Size | Utilization |
| :--- | :--- | :--- |
| **App Partition (app0)** | 1.30 MB (1310720 bytes) | ~820 KB (~63% utilized for Compiled Code + HTML Dashboard) |
| **OTA Upgrade Partition (app1)**| 1.30 MB (1310720 bytes) | Reserved for Over-The-Air wireless firmware updates |
| **NVS Partition (nvs)** | 20 KB (20480 bytes) | Used for WiFi & Tuya Cloud persistent credentials |
| **SPIFFS / LittleFS** | 1.30 MB (1310720 bytes) | User file storage (optional extension space) |
| **OTADATA / PHY / Coredump** | ~80 KB | ESP-IDF system descriptors |
