# Architecture Extensibility & Future Roadmap

The firmware for the ESP32-C3 Super Mini remote control was engineered from the ground up to allow seamless hardware and protocol expansion without structural refactoring.

---

## Modular Design Interfaces

The current codebase decouples event input (buttons), logic routing (FSM), and target execution (Tuya Cloud API). Adding new features requires registering new hardware drivers or API providers into the scheduler and FSM.

```
                           +------------------------+
                           |  Event Scheduler Loop  |
                           +-----------+------------+
                                       |
    +----------------------------------+----------------------------------+
    |                                  |                                  |
+---v--------------------+    +--------v---------------+    +-------------v----------+
|  Existing Components   |    |  Future Protocols      |    |  Future Hardware       |
|  - GPIO9 Button        |    |  - MQTT / Home Assist. |    |  - Relay Outputs       |
|  - Tuya Cloud OpenAPI  |    |  - BLE Mesh            |    |  - OLED / E-Paper Display|
|  - Web Dashboard SPA   |    |  - Matter over Wi-Fi   |    |  - Multi-button Matrix |
+------------------------+    +------------------------+    +------------------------+
```

---

## Proposed Expansion Modules

### 1. Multi-Device & Scene Support
- **Multi-Device Target**: Extend `SystemConfig` in `nvs_config.h` to store arrays of `tuyaDeviceId` maps.
- **Tuya Scenes**: Support calling Tuya Cloud Scene Trigger API (`POST /v1.0/homes/{home_id}/scenes/{scene_id}/trigger`).

### 2. Home Assistant & MQTT Integration
- **Local MQTT Client**: Integrate `PubSubClient` or `AsyncMqttClient` into `scheduler.cpp`.
- **Home Assistant Auto-Discovery**: Publish HA discovery JSON payloads (`homeassistant/switch/remoteswitch/config`) to allow native integration without Tuya cloud latency.

### 3. Physical Relay & Sensor Output
- **Local Relay Driver**: Connect physical relay modules to unused GPIOs (e.g. GPIO2, GPIO3, GPIO4) to allow direct hardware switching during cloud/network outages.
- **Flow & Energy Sensors**: Read pulse counters or power metering ICs on GPIO pins for switch load tracking.

### 4. Bluetooth Low Energy (BLE) Provisioning & Local Remote
- **ESP32-C3 BLE Provider**: Utilize `NimBLE-Arduino` to allow offline phone control via BLE advertising packets.
