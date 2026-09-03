#include "system_manager.h"

#include "core/logging/logger.h"
#include "core/system/system_info.h"
#include "core/monitoring/temperature/temperature_monitor.h"
#include "core/monitoring/uptime/uptime_monitor.h"

void SystemManager::init()
{
    // ----------------------------------------
    // Logging
    // ----------------------------------------

    Logger::init();

    LOG_INFO(
        SYSTEM,
        "CyberSwissKnife System Core starting");

    // ----------------------------------------
    // System information
    // ----------------------------------------

    SystemInfo::print();

    // ----------------------------------------
    // Temperature monitor
    // ----------------------------------------

    if (TemperatureMonitor::init())
    {
        float temperature = 0.0f;

        if (TemperatureMonitor::get_celsius(temperature))
        {
            LOG_INFO(
                SYSTEM,
                "Temperature: %.2f C",
                temperature);
        }
    }

    // ----------------------------------------
    // Uptime
    // ----------------------------------------

    uint64_t uptime_seconds =
        UptimeMonitor::get_seconds();

    uint32_t uptime_days =
        uptime_seconds / 86400;

    uint32_t uptime_hours =
        (uptime_seconds % 86400) / 3600;

    uint32_t uptime_minutes =
        (uptime_seconds % 3600) / 60;

    uint32_t uptime_secs =
        uptime_seconds % 60;

    LOG_INFO(
        SYSTEM,
        "Uptime: %lu days %02lu:%02lu:%02lu",
        (unsigned long)uptime_days,
        (unsigned long)uptime_hours,
        (unsigned long)uptime_minutes,
        (unsigned long)uptime_secs);

    // ----------------------------------------
    // Initialization complete
    // ----------------------------------------

    LOG_INFO(
        SYSTEM,
        "System initialization complete");
}