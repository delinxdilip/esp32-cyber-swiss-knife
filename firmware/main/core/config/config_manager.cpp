#include "config_manager.h"

#include <cstring>

#include "nvs.h"

#include "core/logging/logger.h"

namespace
{
    constexpr const char *NVS_NAMESPACE =
        "config";

    constexpr const char *KEY_AP_SSID =
        "ap_ssid";

    constexpr const char *KEY_AP_PASSWORD =
        "ap_password";

    constexpr const char *KEY_AP_MAX_CONN =
        "ap_max_conn";

    constexpr const char *KEY_AP_CHANNEL =
        "ap_channel";
}

APConfig ConfigManager::ap_config = {};

bool ConfigManager::init()
{
    return load();
}

void ConfigManager::set_defaults()
{
    std::memset(
        &ap_config,
        0,
        sizeof(ap_config));

    std::strncpy(
        ap_config.ssid,
        "CyberSwissKnife",
        sizeof(ap_config.ssid) - 1);

    std::strncpy(
        ap_config.password,
        "csk-local-2026",
        sizeof(ap_config.password) - 1);

    ap_config.max_connections = 4;
    ap_config.channel = 1;
}

bool ConfigManager::validate(
    const APConfig &config)
{
    const size_t ssid_length =
        std::strlen(config.ssid);

    const size_t password_length =
        std::strlen(config.password);

    if (ssid_length == 0 ||
        ssid_length > 32)
    {
        LOG_WARN(
            SYSTEM,
            "Invalid AP SSID length: %u",
            static_cast<unsigned>(
                ssid_length));

        return false;
    }

    if (password_length < 8 ||
        password_length > 64)
    {
        LOG_WARN(
            SYSTEM,
            "Invalid AP password length");

        return false;
    }

    if (config.max_connections < 1 ||
        config.max_connections > 10)
    {
        LOG_WARN(
            SYSTEM,
            "Invalid AP max connections: %u",
            static_cast<unsigned>(
                config.max_connections));

        return false;
    }

    if (config.channel < 1 ||
        config.channel > 13)
    {
        LOG_WARN(
            SYSTEM,
            "Invalid AP channel: %u",
            static_cast<unsigned>(
                config.channel));

        return false;
    }

    return true;
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
            SYSTEM,
            "Failed to open config NVS: %s",
            esp_err_to_name(err));

        return false;
    }

    size_t ssid_size =
        sizeof(ap_config.ssid);

    size_t password_size =
        sizeof(ap_config.password);

    err = nvs_get_str(
        handle,
        KEY_AP_SSID,
        ap_config.ssid,
        &ssid_size);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(handle);

        set_defaults();

        LOG_INFO(
            SYSTEM,
            "No AP configuration found, using defaults");

        return save();
    }

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to load AP SSID: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_get_str(
        handle,
        KEY_AP_PASSWORD,
        ap_config.password,
        &password_size);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to load AP password: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_get_u8(
        handle,
        KEY_AP_MAX_CONN,
        &ap_config.max_connections);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to load AP max connections: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_get_u8(
        handle,
        KEY_AP_CHANNEL,
        &ap_config.channel);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to load AP channel: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    nvs_close(handle);

    if (!validate(ap_config))
    {
        LOG_WARN(
            SYSTEM,
            "Stored AP configuration is invalid; restoring defaults");

        set_defaults();

        return save();
    }

    LOG_INFO(
        SYSTEM,
        "AP configuration loaded: SSID=%s, max_clients=%u, channel=%u",
        ap_config.ssid,
        static_cast<unsigned>(
            ap_config.max_connections),
        static_cast<unsigned>(
            ap_config.channel));

    return true;
}

bool ConfigManager::save()
{
    if (!validate(ap_config))
    {
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
            SYSTEM,
            "Failed to open config NVS for save: %s",
            esp_err_to_name(err));

        return false;
    }

    err = nvs_set_str(
        handle,
        KEY_AP_SSID,
        ap_config.ssid);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to save AP SSID: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_set_str(
        handle,
        KEY_AP_PASSWORD,
        ap_config.password);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to save AP password: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_set_u8(
        handle,
        KEY_AP_MAX_CONN,
        ap_config.max_connections);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to save AP max connections: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_set_u8(
        handle,
        KEY_AP_CHANNEL,
        ap_config.channel);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to save AP channel: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    err = nvs_commit(handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to commit AP configuration: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    nvs_close(handle);

    LOG_INFO(
        SYSTEM,
        "AP configuration saved");

    return true;
}

const APConfig &ConfigManager::get_ap_config()
{
    return ap_config;
}

bool ConfigManager::set_ap_config(
    const APConfig &config)
{
    if (!validate(config))
    {
        return false;
    }

    const APConfig old_config =
        ap_config;

    ap_config = config;

    if (!save())
    {
        ap_config = old_config;

        LOG_ERROR(
            SYSTEM,
            "Failed to persist AP configuration; restored previous configuration");

        return false;
    }

    return true;
}