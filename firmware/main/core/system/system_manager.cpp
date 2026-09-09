#include "system_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"

#include "core/logging/logger.h"
#include "core/system/system_info.h"
#include "core/monitoring/temperature/temperature_monitor.h"
#include "core/monitoring/uptime/uptime_monitor.h"
#include "core/config/config_manager.h"

#include "ui/display/display_manager.h"
#include "ui/display/display_test.h"

#include "wifi/wifi_manager/wifi_manager.h"
#include "network/ap/ap_manager/ap_manager.h"
#include "network/webserver/web_server.h"

void SystemManager::init()
{
    Logger::init();

    /*
     * Development delay.
     *
     * This gives the USB Serial/JTAG console
     * time to reconnect after boot so that
     * startup logs are visible during
     * development.
     */
    vTaskDelay(
        pdMS_TO_TICKS(3000));

    LOG_INFO(
        SYSTEM,
        SYSTEM,
        "CyberSwissKnife System Core starting");

    SystemInfo::print();

    // ----------------------------------------
    // Temperature
    // ----------------------------------------

    if (TemperatureMonitor::init())
    {
        float temperature = 0.0f;

        if (TemperatureMonitor::get_celsius(
                temperature))
        {
            LOG_INFO(
                SYSTEM,
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
        SYSTEM,
        "Uptime: %lu days %02lu:%02lu:%02lu",
        (unsigned long)uptime_days,
        (unsigned long)uptime_hours,
        (unsigned long)uptime_minutes,
        (unsigned long)uptime_secs);

    // ----------------------------------------
    // NVS
    // ----------------------------------------

    esp_err_t nvs_error =
        nvs_flash_init();

    if (nvs_error == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_error == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        LOG_WARN(
            SYSTEM,
            CONFIG,
            "NVS requires erase and reinitialization");

        nvs_error =
            nvs_flash_erase();

        if (nvs_error == ESP_OK)
        {
            nvs_error =
                nvs_flash_init();
        }
    }

    if (nvs_error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            CONFIG,
            "NVS initialization failed: %s",
            esp_err_to_name(nvs_error));

        return;
    }

    LOG_INFO(
        SYSTEM,
        CONFIG,
        "NVS initialized successfully");

    // ----------------------------------------
    // Configuration
    // ----------------------------------------

    if (!ConfigManager::init())
    {
        LOG_ERROR(
            SYSTEM,
            CONFIG,
            "Configuration initialization failed");

        return;
    }

    // ----------------------------------------
    // Display
    // ----------------------------------------

    if (!DisplayManager::initialize())
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Display initialization failed");

        return;
    }

    // ----------------------------------------
    // Display test
    // ----------------------------------------

    DisplayTest::render();

    // ----------------------------------------
    // Wi-Fi
    // ----------------------------------------

    if (!WiFiManager::init())
    {
        LOG_ERROR(
            SYSTEM,
            WIFI,
            "Wi-Fi initialization failed");

        return;
    }

    // ----------------------------------------
    // Access Point
    // ----------------------------------------

    if (!APManager::init())
    {
        LOG_ERROR(
            SYSTEM,
            NETWORK,
            "Access Point initialization failed");

        return;
    }

    // ----------------------------------------
    // HTTP server
    // ----------------------------------------

    if (!WebServer::init())
    {
        LOG_ERROR(
            SYSTEM,
            WEB,
            "HTTP server initialization failed");

        return;
    }

    /*
     * IMPORTANT:
     *
     * Wi-Fi scanning is no longer performed
     * automatically during boot.
     *
     * V1 performs scans only when requested
     * through:
     *
     *     GET /api/wifi/scan
     *
     * This keeps the AP and web server usable
     * after boot and makes the radio behavior
     * easier for beginners to understand.
     */

    LOG_INFO(
        SYSTEM,
        WIFI,
        "Wi-Fi scanning is available on demand");

    LOG_INFO(
        SYSTEM,
        SYSTEM,
        "System initialization complete");
}