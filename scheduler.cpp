/**
 * @file scheduler.cpp
 * @brief Implementation of Lightweight Task Scheduler
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "scheduler.h"

TaskScheduler scheduler;

TaskScheduler::TaskScheduler() : _taskCount(0) {}

void TaskScheduler::begin() {
    // Initialize Task Watchdog Timer
#if defined(ESP_IDF_VERSION_MAJOR) && ESP_IDF_VERSION_MAJOR >= 5
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT_SECONDS * 1000,
        .idle_core_mask = (1 << 0),
        .trigger_panic = true
    };
    esp_task_wdt_init(&wdt_config);
    esp_task_wdt_add(NULL);
#else
    esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL);
#endif

    Serial.printf("Task Watchdog Initialized (%u sec timeout)\n", WDT_TIMEOUT_SECONDS);
}

bool TaskScheduler::addTask(const char* name, unsigned long intervalMs, TaskCallback callback) {
    if (_taskCount >= 8) return false;

    _tasks[_taskCount].name = name;
    _tasks[_taskCount].intervalMs = intervalMs;
    _tasks[_taskCount].lastRunMs = 0;
    _tasks[_taskCount].callback = callback;
    _tasks[_taskCount].enabled = true;
    _taskCount++;
    return true;
}

void TaskScheduler::resetWatchdog() {
    esp_task_wdt_reset();
}

void TaskScheduler::update() {
    unsigned long now = millis();

    for (uint8_t i = 0; i < _taskCount; i++) {
        if (_tasks[i].enabled && _tasks[i].callback != nullptr) {
            if (_tasks[i].intervalMs == 0 || (now - _tasks[i].lastRunMs >= _tasks[i].intervalMs)) {
                _tasks[i].lastRunMs = now;
                _tasks[i].callback();
            }
        }
    }

    resetWatchdog();
}
