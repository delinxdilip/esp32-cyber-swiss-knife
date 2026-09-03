#include "wifi_scanner.h"

#include <string.h>

#include "core/logging/logger.h"

#include "esp_err.h"
#include "esp_wifi.h"

namespace
{
    constexpr uint16_t MAX_NETWORKS = 32;

    WiFiNetwork networks[MAX_NETWORKS];
    uint16_t network_count = 0;

    WiFiSecurity convert_security(
        wifi_auth_mode_t auth_mode)
    {
        switch (auth_mode)
        {
            case WIFI_AUTH_OPEN:
                return WiFiSecurity::OPEN;

            case WIFI_AUTH_WEP:
                return WiFiSecurity::WEP;

            case WIFI_AUTH_WPA_PSK:
                return WiFiSecurity::WPA;

            case WIFI_AUTH_WPA2_PSK:
                return WiFiSecurity::WPA2;

            case WIFI_AUTH_WPA_WPA2_PSK:
                return WiFiSecurity::WPA_WPA2;

            case WIFI_AUTH_WPA3_PSK:
                return WiFiSecurity::WPA3;

            case WIFI_AUTH_WPA2_WPA3_PSK:
                return WiFiSecurity::WPA2_WPA3;

            default:
                return WiFiSecurity::UNKNOWN;
        }
    }

    const char *security_to_string(
        WiFiSecurity security)
    {
        switch (security)
        {
            case WiFiSecurity::OPEN:
                return "OPEN";

            case WiFiSecurity::WEP:
                return "WEP";

            case WiFiSecurity::WPA:
                return "WPA";

            case WiFiSecurity::WPA2:
                return "WPA2";

            case WiFiSecurity::WPA_WPA2:
                return "WPA/WPA2";

            case WiFiSecurity::WPA3:
                return "WPA3";

            case WiFiSecurity::WPA2_WPA3:
                return "WPA2/WPA3";

            default:
                return "UNKNOWN";
        }
    }
}

bool WiFiScanner::scan()
{
    LOG_INFO(
        WIFI,
        "Starting Wi-Fi scan");

    // ----------------------------------------
    // Clear previous results
    // ----------------------------------------

    network_count = 0;

    // ----------------------------------------
    // Set Wi-Fi station mode
    // ----------------------------------------

    esp_err_t err =
        esp_wifi_set_mode(WIFI_MODE_APSTA);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Failed to set Wi-Fi APSTA mode: %s",
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
            WIFI,
            "Failed to start Wi-Fi: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Configure scan
    // ----------------------------------------

    wifi_scan_config_t scan_config = {};

    scan_config.ssid = nullptr;
    scan_config.bssid = nullptr;
    scan_config.channel = 0;
    scan_config.show_hidden = true;

    // ----------------------------------------
    // Start scan
    // ----------------------------------------

    err = esp_wifi_scan_start(
        &scan_config,
        true);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Wi-Fi scan failed: %s",
            esp_err_to_name(err));

        return false;
    }

    // ----------------------------------------
    // Get network count
    // ----------------------------------------

    uint16_t discovered_count = 0;

    err = esp_wifi_scan_get_ap_num(
        &discovered_count);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Failed to get network count: %s",
            esp_err_to_name(err));

        return false;
    }

    LOG_INFO(
        WIFI,
        "Networks found: %u",
        discovered_count);

    if (discovered_count == 0)
    {
        return true;
    }

    // ----------------------------------------
    // Limit results to our storage capacity
    // ----------------------------------------

    uint16_t result_capacity =
        discovered_count;

    if (result_capacity > MAX_NETWORKS)
    {
        result_capacity = MAX_NETWORKS;

        LOG_WARN(
            WIFI,
            "Network count exceeds storage capacity, keeping first %u",
            MAX_NETWORKS);
    }

    // ----------------------------------------
    // Allocate temporary ESP-IDF records
    // ----------------------------------------

    wifi_ap_record_t *records =
        new wifi_ap_record_t[discovered_count];

    if (records == nullptr)
    {
        LOG_ERROR(
            WIFI,
            "Failed to allocate Wi-Fi scan results");

        return false;
    }

    uint16_t result_count = discovered_count;

    // ----------------------------------------
    // Retrieve scan results
    // ----------------------------------------

    err = esp_wifi_scan_get_ap_records(
        &result_count,
        records);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Failed to retrieve Wi-Fi scan results: %s",
            esp_err_to_name(err));

        delete[] records;

        return false;
    }

    // ----------------------------------------
    // Convert and store results
    // ----------------------------------------

    for (uint16_t i = 0;
         i < result_count && i < result_capacity;
         i++)
    {
        const wifi_ap_record_t &record =
            records[i];

        WiFiNetwork &network =
            networks[network_count];

        memset(
            &network,
            0,
            sizeof(WiFiNetwork));

        // SSID
        memcpy(
            network.ssid,
            record.ssid,
            sizeof(network.ssid) - 1);

        network.ssid[
            sizeof(network.ssid) - 1] = '\0';

        // BSSID
        memcpy(
            network.bssid,
            record.bssid,
            sizeof(network.bssid));

        // Signal strength
        network.rssi =
            record.rssi;

        // Channel
        network.channel =
            record.primary;

        // Hidden network
        network.hidden =
            (record.ssid[0] == '\0');

        // Security
        network.security =
            convert_security(record.authmode);

        network_count++;

        // ----------------------------------------
        // Log network
        // ----------------------------------------

        LOG_INFO(
            WIFI,
            "[%u] SSID: %s | RSSI: %d dBm | CH: %u | Security: %s | Hidden: %s",
            network_count,
            network.ssid,
            network.rssi,
            network.channel,
            security_to_string(network.security),
            network.hidden ? "YES" : "NO");
    }

    // ----------------------------------------
    // Cleanup
    // ----------------------------------------

    delete[] records;

    LOG_INFO(
        WIFI,
        "Wi-Fi scan results stored: %u",
        network_count);

    return true;
}

uint16_t WiFiScanner::get_count()
{
    return network_count;
}

const WiFiNetwork *WiFiScanner::get_network(
    uint16_t index)
{
    if (index >= network_count)
    {
        return nullptr;
    }

    return &networks[index];
}