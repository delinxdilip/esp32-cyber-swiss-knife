#include "ap_manager.h"

#include <cstring>

#include "esp_wifi.h"
#include "esp_netif.h"

#include "core/logging/logger.h"

namespace
{
    esp_netif_t *ap_netif = nullptr;
    bool initialized = false;
}

bool APManager::init()
{
    if (initialized)
    {
        LOG_WARN(
            NETWORK,
            "Access Point already initialized");

        return true;
    }

    const APConfig &config =
        ConfigManager::get_ap_config();

    if (!ConfigManager::validate(config))
    {
        LOG_ERROR(
            NETWORK,
            "Invalid AP configuration");

        return false;
    }

    /*
     * Create the default AP network interface
     * only once.
     */
    if (ap_netif == nullptr)
    {
        ap_netif =
            esp_netif_create_default_wifi_ap();

        if (ap_netif == nullptr)
        {
            LOG_ERROR(
                NETWORK,
                "Failed to create AP network interface");

            return false;
        }
    }

    /*
     * Configure the AP.
     */
    if (!apply_config(config))
    {
        LOG_ERROR(
            NETWORK,
            "Failed to apply AP configuration");

        return false;
    }

    /*
     * IMPORTANT:
     *
     * esp_wifi_init() only initializes the Wi-Fi
     * driver. The radio/AP does not actually start
     * until esp_wifi_start() is called.
     */
    esp_err_t result =
        esp_wifi_start();

    if (result != ESP_OK &&
        result != ESP_ERR_WIFI_CONN)
    {
        /*
         * ESP_ERR_WIFI_CONN isn't normally expected
         * here, but don't treat it as a fatal
         * initialization failure.
         */
        LOG_ERROR(
            NETWORK,
            "Failed to start Wi-Fi: %s",
            esp_err_to_name(result));

        return false;
    }

    initialized = true;

    LOG_INFO(
        NETWORK,
        "Access Point started successfully");

    LOG_INFO(
        NETWORK,
        "AP SSID: %s",
        config.ssid);

    LOG_INFO(
        NETWORK,
        "AP IP: 192.168.4.1");

    return true;
}

bool APManager::apply_config(
    const APConfig &config)
{
    if (!ConfigManager::validate(config))
    {
        LOG_ERROR(
            NETWORK,
            "Attempted to apply invalid AP configuration");

        return false;
    }

    if (ap_netif == nullptr)
    {
        LOG_ERROR(
            NETWORK,
            "AP network interface is not initialized");

        return false;
    }

    wifi_config_t ap_config = {};

    std::strncpy(
        reinterpret_cast<char *>(
            ap_config.ap.ssid),
        config.ssid,
        sizeof(ap_config.ap.ssid) - 1);

    ap_config.ap.ssid[
        sizeof(ap_config.ap.ssid) - 1] =
        '\0';

    std::strncpy(
        reinterpret_cast<char *>(
            ap_config.ap.password),
        config.password,
        sizeof(ap_config.ap.password) - 1);

    ap_config.ap.password[
        sizeof(ap_config.ap.password) - 1] =
        '\0';

    ap_config.ap.ssid_len =
        static_cast<uint8_t>(
            std::strlen(config.ssid));

    ap_config.ap.channel =
        config.channel;

    ap_config.ap.max_connection =
        config.max_connections;

    ap_config.ap.authmode =
        WIFI_AUTH_WPA2_PSK;

    /*
     * Protected Management Frames:
     *
     * Capable = true
     * Required = false
     *
     * This keeps the AP compatible with
     * normal phones/laptops while supporting
     * PMF-capable clients.
     */
    ap_config.ap.pmf_cfg.capable = true;
    ap_config.ap.pmf_cfg.required = false;

    /*
     * APSTA allows the ESP32 to operate as an
     * Access Point while retaining the ability
     * to use station mode later.
     */
    esp_err_t result =
        esp_wifi_set_mode(
            WIFI_MODE_APSTA);

    if (result != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to set APSTA mode: %s",
            esp_err_to_name(result));

        return false;
    }

    result =
        esp_wifi_set_config(
            WIFI_IF_AP,
            &ap_config);

    if (result != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to configure AP: %s",
            esp_err_to_name(result));

        return false;
    }

    LOG_INFO(
        NETWORK,
        "AP configuration applied: SSID=%s, channel=%u, max_clients=%u",
        config.ssid,
        static_cast<unsigned int>(
            config.channel),
        static_cast<unsigned int>(
            config.max_connections));

    return true;
}

bool APManager::is_initialized()
{
    return initialized;
}