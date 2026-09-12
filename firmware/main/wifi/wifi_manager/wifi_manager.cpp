#include "wifi_manager.h"

#include "core/logging/logger.h"

#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

bool WiFiManager::init()
{
    // ----------------------------------------
    // NVS
    // ----------------------------------------

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        LOG_WARN(
            ACTIVITY,
            WIFI,
            "NVS requires erase and reinitialization");

        err = nvs_flash_erase();

        if (err != ESP_OK)
        {
            LOG_ERROR(
                ACTIVITY,
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
            ACTIVITY,
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
            ACTIVITY,
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
            ACTIVITY,
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
            ACTIVITY,
            WIFI,
            "Wi-Fi driver initialization failed: %s",
            esp_err_to_name(err));

        return false;
    }

    LOG_INFO(
        ACTIVITY,
        WIFI,
        "Wi-Fi manager initialized");

    return true;
}