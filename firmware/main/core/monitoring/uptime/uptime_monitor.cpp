#include "uptime_monitor.h"

#include "esp_timer.h"

uint64_t UptimeMonitor::get_seconds()
{
    int64_t uptime_us = esp_timer_get_time();

    return static_cast<uint64_t>(
        uptime_us / 1000000ULL);
}