#include "config_manager.h"

#include <string.h>

#include "core/logging/logger.h"

#include "nvs.h"
#include "nvs_flash.h"

namespace
{
    constexpr char NVS_NAMESPACE[] = "config";

    constexpr char KEY_SSID[] = "ap_ssid";
    constexpr char KEY_PASSWORD[] = "ap_password";
    constexpr char KEY_MAX_CONN[] = "ap_max_conn";
    constexpr char KEY_CHANNEL[] = "ap_channel";

    constexpr char DEFAULT_SSID[] =
        "CyberSwissKnife";

    constexpr char DEFAULT_PASSWORD[] =
        "csk-local-2026";

    constexpr uint8_t DEFAULT_MAX_CONNECTIONS = 4;
    constexpr uint8_t DEFAULT_CHANNEL = 1;

    void set_defaults(APConfig &config)
    {
        memset(
            &config,
            0,
            sizeof(APConfig));

        strncpy(
            config.ssid,
            DEFAULT_SSID,
            sizeof(config.ssid) - 1);

        strncpy(
            config.password,
            DEFAULT_PASSWORD,
            sizeof(config.password) - 1);

        config.max_connections =
            DEFAULT_MAX_CONNECTIONS;

        config.channel =
            DEFAULT_CHANNEL;
    }

    bool validate_config(
        const APConfig &config)
    {
        if (config.ssid[0] == '\0')
        {
            LOG_ERROR(
                NETWORK,
                "AP SSID cannot be empty");

            return false;
        }

        if (strlen(config.ssid) > 32)
        {
            LOG_ERROR(
                NETWORK,
                "AP SSID exceeds 32 characters");

            return false;
        }

        if (strlen(config.password) < 8)
        {
            LOG_ERROR(
                NETWORK,
                "AP password must be at least 8 characters");

            return false;
        }

        if (strlen(config.password) > 64)
        {
            LOG_ERROR(
                NETWORK,
                "AP password exceeds 64 characters");

            return false;
        }

        if (config.max_connections == 0)
        {
            LOG_ERROR(
                NETWORK,
                "AP max connections cannot be zero");

            return false;
        }

        /*
         * ESP32 SoftAP supports up to 10 stations
         * by default/configuration limits.
         */
        if (config.max_connections > 10)
        {
            LOG_ERROR(
                NETWORK,
                "AP max connections cannot exceed 10");

            return false;
        }

        if (config.channel < 1 ||
            config.channel > 13)
        {
            LOG_ERROR(
                NETWORK,
                "AP channel must be between 1 and 13");

            return false;
        }

        return true;
    }
}

APConfig ConfigManager::ap_config = {};

bool ConfigManager::init()
{
    LOG_INFO(
        NETWORK,
        "Initializing configuration manager");

    /*
     * WiFiManager currently initializes NVS.
     * ConfigManager assumes NVS is already initialized.
     */

    return load();
}

bool ConfigManager::load()
{
    nvs_handle_t handle;

    esp_err_t err =
        nvs_open(
            NVS_NAMESPACE,
            NVS_READWRITE,
            &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to open configuration NVS: %s",
            esp_err_to_name(err));

        return false;
    }

    size_t ssid_length =
        sizeof(ap_config.ssid);

    size_t password_length =
        sizeof(ap_config.password);

    err =
        nvs_get_str(
            handle,
            KEY_SSID,
            ap_config.ssid,
            &ssid_length);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        LOG_INFO(
            NETWORK,
            "No saved configuration found");

        set_defaults(ap_config);

        nvs_close(handle);

        if (!save())
        {
            LOG_ERROR(
                NETWORK,
                "Failed to save default configuration");

            return false;
        }

        LOG_INFO(
            NETWORK,
            "Default configuration created");

        return true;
    }

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to load AP SSID: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err =
        nvs_get_str(
            handle,
            KEY_PASSWORD,
            ap_config.password,
            &password_length);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to load AP password: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    uint8_t value = 0;

    err =
        nvs_get_u8(
            handle,
            KEY_MAX_CONN,
            &value);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to load AP max connections: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    ap_config.max_connections =
        value;

    err =
        nvs_get_u8(
            handle,
            KEY_CHANNEL,
            &value);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to load AP channel: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    ap_config.channel =
        value;

    nvs_close(handle);

    if (!validate_config(ap_config))
    {
        LOG_ERROR(
            NETWORK,
            "Saved configuration is invalid");

        return false;
    }

    LOG_INFO(
        NETWORK,
        "Configuration loaded from NVS");

    LOG_INFO(
        NETWORK,
        "AP SSID: %s",
        ap_config.ssid);

    LOG_INFO(
        NETWORK,
        "AP max connections: %u",
        ap_config.max_connections);

    LOG_INFO(
        NETWORK,
        "AP channel: %u",
        ap_config.channel);

    return true;
}

bool ConfigManager::save()
{
    if (!validate_config(ap_config))
    {
        LOG_ERROR(
            NETWORK,
            "Configuration validation failed");

        return false;
    }

    nvs_handle_t handle;

    esp_err_t err =
        nvs_open(
            NVS_NAMESPACE,
            NVS_READWRITE,
            &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to open configuration NVS: %s",
            esp_err_to_name(err));

        return false;
    }

    err =
        nvs_set_str(
            handle,
            KEY_SSID,
            ap_config.ssid);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to save AP SSID: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err =
        nvs_set_str(
            handle,
            KEY_PASSWORD,
            ap_config.password);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to save AP password: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err =
        nvs_set_u8(
            handle,
            KEY_MAX_CONN,
            ap_config.max_connections);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to save AP max connections: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err =
        nvs_set_u8(
            handle,
            KEY_CHANNEL,
            ap_config.channel);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to save AP channel: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_commit(handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to commit configuration: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    nvs_close(handle);

    LOG_INFO(
        NETWORK,
        "Configuration saved to NVS");

    return true;
}

const APConfig &ConfigManager::get_ap_config()
{
    return ap_config;
}

bool ConfigManager::set_ap_config(
    const APConfig &config)
{
    if (!validate_config(config))
    {
        LOG_ERROR(
            NETWORK,
            "Rejected invalid AP configuration");

        return false;
    }

    ap_config = config;

    return save();
}