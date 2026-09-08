#include "wifi_manager.h"

#include "core/logging/logger.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

namespace
{
    bool initialized = false;
}

bool WiFiManager::init()
{
    if (initialized)
    {
        LOG_DEBUG(
            WIFI,
            "Wi-Fi manager already initialized");

        return true;
    }

    // ----------------------------------------
    // NVS
    // ----------------------------------------

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        LOG_WARN(
            WIFI,
            "NVS requires erase and reinitialization");

        err = nvs_flash_erase();

        if (err != ESP_OK)
        {
            LOG_ERROR(
                WIFI,
                "Failed to erase NVS: %s",
                esp_err_to_name(err));

            return false;
        }

        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "NVS initialization failed: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Network interface
    // ----------------------------------------

    err = esp_netif_init();

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        LOG_ERROR(
            WIFI,
            "Network interface initialization failed: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Event loop
    // ----------------------------------------

    err = esp_event_loop_create_default();

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        LOG_ERROR(
            WIFI,
            "Event loop initialization failed: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Wi-Fi driver
    // ----------------------------------------

    wifi_init_config_t config =
        WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&config);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Wi-Fi driver initialization failed: %s",
            esp_err_to_name(err));

        return false;
    }

    initialized = true;

    LOG_INFO(
        WIFI,
        "Wi-Fi manager initialized");

    return true;
}