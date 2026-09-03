#include "ap_manager.h"

#include <string.h>

#include "core/logging/logger.h"

#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"

namespace
{
    constexpr char AP_SSID[] = "CyberSwissKnife";
    constexpr char AP_PASSWORD[] = "csk-local-2026";

    constexpr uint8_t AP_CHANNEL = 1;
    constexpr uint8_t AP_MAX_CONNECTIONS = 4;

    esp_netif_t *ap_netif = nullptr;
    bool initialized = false;
}

bool APManager::init()
{
    if (initialized)
    {
        LOG_DEBUG(
            NETWORK,
            "Access Point already initialized");

        return true;
    }

    // ----------------------------------------
    // Create default AP network interface
    // ----------------------------------------

    ap_netif =
        esp_netif_create_default_wifi_ap();

    if (ap_netif == nullptr)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to create AP network interface");

        return false;
    }

    // ----------------------------------------
    // Configure Access Point
    // ----------------------------------------

    wifi_config_t ap_config = {};

    strncpy(
        reinterpret_cast<char *>(ap_config.ap.ssid),
        AP_SSID,
        sizeof(ap_config.ap.ssid));

    strncpy(
        reinterpret_cast<char *>(ap_config.ap.password),
        AP_PASSWORD,
        sizeof(ap_config.ap.password));

    ap_config.ap.ssid_len =
        strlen(AP_SSID);

    ap_config.ap.channel =
        AP_CHANNEL;

    ap_config.ap.max_connection =
        AP_MAX_CONNECTIONS;

    ap_config.ap.authmode =
        WIFI_AUTH_WPA2_PSK;

    ap_config.ap.pmf_cfg.capable = true;
    ap_config.ap.pmf_cfg.required = false;

    // ----------------------------------------
    // AP + Station mode
    // ----------------------------------------

    esp_err_t err =
        esp_wifi_set_mode(WIFI_MODE_APSTA);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to set APSTA mode: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Apply AP configuration
    // ----------------------------------------

    err =
        esp_wifi_set_config(
            WIFI_IF_AP,
            &ap_config);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to configure Access Point: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Start Wi-Fi
    // ----------------------------------------

    err = esp_wifi_start();

    if (err != ESP_OK &&
        err != ESP_ERR_WIFI_STATE)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to start Wi-Fi: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Get AP IP address
    // ----------------------------------------

    esp_netif_ip_info_t ip_info = {};

    err =
        esp_netif_get_ip_info(
            ap_netif,
            &ip_info);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            NETWORK,
            "Failed to get AP IP address: %s",
            esp_err_to_name(err));

        return false;
    }

    LOG_INFO(
        NETWORK,
        "Access Point initialized");

    LOG_INFO(
        NETWORK,
        "SSID: %s",
        AP_SSID);

    LOG_INFO(
        NETWORK,
        "Channel: %u",
        AP_CHANNEL);

    LOG_INFO(
        NETWORK,
        "IP address: " IPSTR,
        IP2STR(&ip_info.ip));

    LOG_INFO(
        NETWORK,
        "Gateway: " IPSTR,
        IP2STR(&ip_info.gw));

    LOG_INFO(
        NETWORK,
        "Subnet: " IPSTR,
        IP2STR(&ip_info.netmask));

    initialized = true;

    return true;
}