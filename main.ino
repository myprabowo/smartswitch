/**
 * @file main.ino
 * @brief ESP32-C3 Super Mini Tuya Smart Switch Remote Control Firmware
 * @author Embedded Systems Team
 * @version 1.0.0
 * 
 * Hardware: ESP32-C3 Super Mini
 * Button: GPIO9 (BOOT Button, Active LOW)
 * LED: GPIO8 (Onboard LED, Active LOW)
 */

#include "config.h"
#include "utils.h"
#include "nvs_config.h"
#include "led.h"
#include "button.h"
#include "tuya_api.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "health_monitor.h"
#include "web_server.h"
#include "scheduler.h"
#include "state_machine.h"

// Forward Declarations
void handleButtonGestures();
void handleTuyaShortPress();
void handleTuyaForceOn();
void handleTuyaForceOff();

void setup() {
    // 1. Initialize Serial Communication
    Serial.begin(115200);
    delay(300); // Brief hardware startup pause for USB Serial CDC

    Serial.println("\n----------------------------------");
    Serial.println("Booting...");
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
    Serial.printf("Hardware Model:   %s\n", HARDWARE_MODEL);
    Serial.printf("Reset Reason:     %s\n", Utils::getResetReasonString(esp_reset_reason()).c_str());
    Serial.println("----------------------------------");

    // 2. Initialize Core Hardware Peripherals
    ledController.begin();
    buttonHandler.begin();
    
    // 3. Load NVS Configuration & Increment Boot Count
    nvsConfigManager.begin();
    uint32_t crashes = nvsConfigManager.incrementBootCount();
    Serial.printf("Boot Count: %u | Crash Tracker: %u\n", nvsConfigManager.getBootCount(), crashes);

    // 4. Initialize Self Diagnostics & Logging Subsystem
    healthMonitor.runBootDiagnostics();

    // 5. Initialize Tuya Client & NTP
    tuyaApiClient.begin();
    ntpManager.begin();

    // 6. Connect to WiFi or Launch Captive Portal Provisioning
    Serial.println("Connecting WiFi...");
    bool wifiOk = customWiFiManager.begin(crashes >= SAFE_MODE_CRASH_LIMIT);
    if (!wifiOk) {
        Serial.println("WiFi Provisioning Mode active. Access http://192.168.4.1 to configure.");
    }

    // 7. Start Web Server & REST API
    embeddedWebServer.begin();

    // 8. Initialize Task Scheduler & Watchdog
    scheduler.begin();
    
    // Register Periodic Tasks in Scheduler
    scheduler.addTask("WiFiCheck", WIFI_CHECK_INTERVAL_MS, []() {
        customWiFiManager.update();
    });

    scheduler.addTask("HealthLog", HEALTH_LOG_INTERVAL_MS, []() {
        healthMonitor.update();
    });

    scheduler.addTask("NTPUpdate", 10000, []() {
        ntpManager.update();
    });

    // 9. Initialize State Machine
    systemFSM.begin();

    // 10. Initial Switch Status Reading
    if (customWiFiManager.isConnected()) {
        bool initialStatus = false;
        if (tuyaApiClient.getSwitchStatus(initialStatus) == TUYA_SUCCESS) {
            embeddedWebServer.setCachedPumpState(initialStatus);
        }
    }
}

void loop() {
    // Master Non-Blocking Execution Loop
    scheduler.update();
    buttonHandler.update();
    ledController.update();
    systemFSM.update();
    embeddedWebServer.update();

    // Handle Physical Button Events
    handleButtonGestures();
}

void handleButtonGestures() {
    ButtonEvent evt = buttonHandler.update();
    if (evt == BUTTON_EVENT_NONE) return;

    switch (evt) {
        case BUTTON_EVENT_SHORT_PRESS:
            Serial.println("\n[Button] Short Press (<500ms) Detected");
            handleTuyaShortPress();
            break;

        case BUTTON_EVENT_DOUBLE_CLICK:
            Serial.println("\n[Button] Double Click Detected -> Force OFF");
            handleTuyaForceOff();
            break;

        case BUTTON_EVENT_LONG_PRESS:
            Serial.println("\n[Button] Long Press (>2s) Detected -> Force ON");
            handleTuyaForceOn();
            break;

        case BUTTON_EVENT_VERY_LONG_PRESS:
            Serial.println("\n[Button] Very Long Press (>8s) -> Erasing WiFi Credentials...");
            healthMonitor.logWarn("Button reset trigger: Erasing WiFi credentials.");
            ledController.setPattern(LED_PATTERN_RAPID_BLINK);
            customWiFiManager.resetCredentialsAndRestart();
            break;

        case BUTTON_EVENT_FACTORY_RESET:
            Serial.println("\n[Button] Factory Reset (>15s) -> Wiping NVS & Restarting...");
            healthMonitor.logWarn("Button reset trigger: Full Factory Reset.");
            systemFSM.transitionTo(STATE_FACTORY_RESET);
            break;

        default:
            break;
    }
}

void handleTuyaShortPress() {
    systemFSM.transitionTo(STATE_SENDING_COMMAND);
    bool newState = false;
    TuyaCommandResult res = tuyaApiClient.toggleSwitch(newState);

    if (res == TUYA_SUCCESS) {
        embeddedWebServer.setCachedPumpState(newState);
        ledController.setPattern(LED_PATTERN_ONE_LONG_BLINK);
        systemFSM.transitionTo(STATE_READY);
    } else {
        healthMonitor.logError("Short Press Command Failed: " + tuyaApiClient.getLastResponseMessage());
        ledController.setPattern(LED_PATTERN_RAPID_BLINK);
        systemFSM.transitionTo(STATE_ERROR);
    }
}

void handleTuyaForceOn() {
    systemFSM.transitionTo(STATE_SENDING_COMMAND);
    TuyaCommandResult res = tuyaApiClient.setSwitchStatus(true);

    if (res == TUYA_SUCCESS) {
        embeddedWebServer.setCachedPumpState(true);
        ledController.setPattern(LED_PATTERN_ONE_LONG_BLINK);
        systemFSM.transitionTo(STATE_READY);
    } else {
        healthMonitor.logError("Force ON Command Failed: " + tuyaApiClient.getLastResponseMessage());
        ledController.setPattern(LED_PATTERN_RAPID_BLINK);
        systemFSM.transitionTo(STATE_ERROR);
    }
}

void handleTuyaForceOff() {
    systemFSM.transitionTo(STATE_SENDING_COMMAND);
    TuyaCommandResult res = tuyaApiClient.setSwitchStatus(false);

    if (res == TUYA_SUCCESS) {
        embeddedWebServer.setCachedPumpState(false);
        ledController.setPattern(LED_PATTERN_ONE_LONG_BLINK);
        systemFSM.transitionTo(STATE_READY);
    } else {
        healthMonitor.logError("Force OFF Command Failed: " + tuyaApiClient.getLastResponseMessage());
        ledController.setPattern(LED_PATTERN_RAPID_BLINK);
        systemFSM.transitionTo(STATE_ERROR);
    }
}
