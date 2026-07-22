/**
 * @file scheduler.h
 * @brief Lightweight Non-Blocking Task Scheduler for ESP32
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef PUMP_REMOTE_SCHEDULER_H
#define PUMP_REMOTE_SCHEDULER_H

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config.h"

typedef void (*TaskCallback)();

struct Task {
    const char* name;
    unsigned long intervalMs;
    unsigned long lastRunMs;
    TaskCallback callback;
    bool enabled;
};

class TaskScheduler {
public:
    TaskScheduler();

    /**
     * @brief Initializes Hardware Watchdog and Scheduler.
     */
    void begin();

    /**
     * @brief Non-blocking updater called inside loop().
     */
    void update();

    /**
     * @brief Registers a new periodic task.
     */
    bool addTask(const char* name, unsigned long intervalMs, TaskCallback callback);

private:
    Task _tasks[8];
    uint8_t _taskCount;

    void resetWatchdog();
};

extern TaskScheduler scheduler;

#endif // PUMP_REMOTE_SCHEDULER_H
