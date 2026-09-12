#include "ap_manager.h"

#include <cstring>

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_event.h"

#include "core/logging/logger.h"

namespace
{
    esp_netif_t *ap_netif = nullptr;

    bool initialized = false;
    bool running = false;
    bool event_handler_registered = false;

    uint8_t client_count = 0;

    void wifi_event_handler(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data)
    {
        (void)arg;
        (void)event_data;

        if (event_base != WIFI_EVENT)
        {
            return;
        }

        switch (event_id)
        {
        case WIFI_EVENT_AP_START:
        {
            running = true;
            client_count = 0;

            LOG_INFO(
                SYSTEM,
                NETWORK,
                "Access Point started");

            break;
        }

        case WIFI_EVENT_AP_STOP:
        {
            running = false;
            client_count = 0;

            LOG_INFO(
                SYSTEM,
                NETWORK,
                "Access Point stopped");

            break;
        }

        case WIFI_EVENT_AP_STACONNECTED:
        {
            if (client_count < UINT8_MAX)
            {
                client_count++;
            }

            LOG_INFO(
                SYSTEM,
                NETWORK,
                "Station connected to AP, clients=%u",
                static_cast<unsigned int>(client_count));

            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            if (client_count > 0)
            {
                client_count--;
            }

            LOG_INFO(
                SYSTEM,
                NETWORK,
                "Station disconnected from AP, clients=%u",
                static_cast<unsigned int>(client_count));

            break;
        }

        default:
            break;
        }
    }
}

bool APManager::init()
{
    LOG_INFO(
        SYSTEM,
        NETWORK,
        "APManager::init() entered");

    if (initialized)
    {
        LOG_WARN(
            SYSTEM,
            NETWORK,
            "Access Point already initialized");

        return true;
    }

    const APConfig &config =
        ConfigManager::get_ap_config();

    LOG_INFO(
        SYSTEM,
        NETWORK,
        "Loaded AP configuration: SSID=%s, channel=%u, max_clients=%u, enabled=%s",
        config.ssid,
        static_cast<unsigned int>(config.channel),
        static_cast<unsigned int>(config.max_connections),
        config.enabled ? "true" : "false");

    if (!config.enabled)
    {
        LOG_WARN(
            SYSTEM,
            NETWORK,
            "Access Point is disabled in configuration");

        running = false;
        return true;
    }

    if (!ConfigManager::validate(config))
    {
        LOG_ERROR(
            SYSTEM,
            NETWORK,
            "Invalid AP configuration");

        return false;
    }

    /*
     * Create the default AP network interface only once.
     */
    if (ap_netif == nullptr)
    {
        LOG_INFO(
            SYSTEM,
            NETWORK,
            "Creating default AP network interface");

        ap_netif =
            esp_netif_create_default_wifi_ap();

        if (ap_netif == nullptr)
        {
            LOG_ERROR(
                SYSTEM,
                NETWORK,
                "Failed to create AP network interface");

            return false;
        }

        LOG_INFO(
            SYSTEM,
            NETWORK,
            "AP network interface created");
    }

    /*
     * Configure the AP.
     */
    if (!apply_config(config))
    {
        LOG_ERROR(
            SYSTEM,
            NETWORK,
            "Failed to apply AP configuration");

        return false;
    }

    /*
     * Register Wi-Fi event handler before starting
     * the Wi-Fi driver.
     */
    if (!event_handler_registered)
    {
        esp_err_t result =
            esp_event_handler_register(
                WIFI_EVENT,
                ESP_EVENT_ANY_ID,
                &wifi_event_handler,
                nullptr);

        if (result != ESP_OK)
        {
            LOG_ERROR(
                SYSTEM,
                NETWORK,
                "Failed to register Wi-Fi event handler: %s",
                esp_err_to_name(result));

            return false;
        }

        event_handler_registered = true;
    }

    /*
     * Start the Wi-Fi driver.
     */
    LOG_INFO(
        SYSTEM,
        NETWORK,
        "Starting Wi-Fi driver");

    esp_err_t result =
        esp_wifi_start();

    if (result != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            NETWORK,
            "Failed to start Wi-Fi: %s",
            esp_err_to_name(result));

        return false;
    }

    /*
     * esp_wifi_start() succeeded, and the APSTA mode
     * was configured successfully.
     *
     * Set this as a fallback in case the event callback
     * is delivered slightly later.
     */
    running = true;
    client_count = 0;
    initialized = true;

    LOG_INFO(
        SYSTEM,
        NETWORK,
        "Wi-Fi driver started successfully");

    LOG_INFO(
        SYSTEM,
        NETWORK,
        "Access Point configured successfully");

    LOG_INFO(
        SYSTEM,
        NETWORK,
        "AP SSID: %s",
        config.ssid);

    LOG_INFO(
        SYSTEM,
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
            SYSTEM,
            NETWORK,
            "Attempted to apply invalid AP configuration");

        return false;
    }

    if (ap_netif == nullptr)
    {
        LOG_ERROR(
            SYSTEM,
            NETWORK,
            "AP network interface is not initialized");

        return false;
    }

    wifi_config_t ap_config = {};

    std::strncpy(
        reinterpret_cast<char *>(ap_config.ap.ssid),
        config.ssid,
        sizeof(ap_config.ap.ssid) - 1);

    ap_config.ap.ssid[
        sizeof(ap_config.ap.ssid) - 1] = '\0';

    std::strncpy(
        reinterpret_cast<char *>(ap_config.ap.password),
        config.password,
        sizeof(ap_config.ap.password) - 1);

    ap_config.ap.password[
        sizeof(ap_config.ap.password) - 1] = '\0';

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
     * Capable  = true
     * Required = false
     */
    ap_config.ap.pmf_cfg.capable = true;
    ap_config.ap.pmf_cfg.required = false;

    /*
     * Preserve APSTA mode.
     *
     * The ESP32 can operate as an Access Point
     * while retaining station-mode capability.
     */
    esp_err_t result =
        esp_wifi_set_mode(WIFI_MODE_APSTA);

    if (result != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
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
            SYSTEM,
            NETWORK,
            "Failed to configure AP: %s",
            esp_err_to_name(result));

        return false;
    }

    LOG_INFO(
        SYSTEM,
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

bool APManager::is_running()
{
    return running;
}

uint8_t APManager::get_client_count()
{
    return client_count;
}