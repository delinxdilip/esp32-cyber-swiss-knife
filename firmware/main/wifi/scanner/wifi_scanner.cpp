#include "wifi_scanner.h"

#include <string.h>

#include "core/logging/logger.h"

#include "esp_err.h"
#include "esp_wifi.h"

namespace
{
    constexpr uint16_t MAX_NETWORKS = 32;

    WiFiNetwork networks[MAX_NETWORKS] = {};

    uint16_t network_count = 0;

    bool scanning = false;

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
}

bool WiFiScanner::scan()
{
    if (scanning)
    {
        LOG_WARN(
            WIFI,
            "Wi-Fi scan already in progress");

        return false;
    }

    scanning = true;

    LOG_INFO(
        WIFI,
        "Starting Wi-Fi scan");

    /*
     * The ESP32-S3 has one Wi-Fi radio.
     *
     * A scan temporarily moves that radio
     * between Wi-Fi channels. Because our AP
     * uses the same radio, clients may briefly
     * lose connectivity during the scan.
     *
     * This is intentional in V1.
     *
     * We therefore perform scans only when
     * explicitly requested instead of scanning
     * continuously in the background.
     */

    wifi_scan_config_t scan_config = {};

    scan_config.ssid = nullptr;
    scan_config.bssid = nullptr;
    scan_config.channel = 0;
    scan_config.show_hidden = true;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    scan_config.scan_time.active.min = 100;
    scan_config.scan_time.active.max = 300;

    /*
     * APSTA allows the ESP32 to keep the
     * Wi-Fi interface configured for both
     * station and access-point operation.
     */
    esp_err_t err =
        esp_wifi_set_mode(WIFI_MODE_APSTA);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Failed to set APSTA mode: %s",
            esp_err_to_name(err));

        scanning = false;
        return false;
    }

    /*
     * Start a blocking scan.
     *
     * true means this function waits until
     * the scan is complete.
     */
    err =
        esp_wifi_scan_start(
            &scan_config,
            true);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Wi-Fi scan failed to start: %s",
            esp_err_to_name(err));

        scanning = false;
        return false;
    }

    uint16_t discovered_count = 0;

    err =
        esp_wifi_scan_get_ap_num(
            &discovered_count);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WIFI,
            "Failed to get Wi-Fi scan count: %s",
            esp_err_to_name(err));

        scanning = false;
        return false;
    }

    LOG_INFO(
        WIFI,
        "Networks discovered: %u",
        discovered_count);

    /*
     * Limit the number of stored networks.
     *
     * The scanner intentionally keeps a fixed
     * memory footprint for V1.
     */
    uint16_t stored_count =
        discovered_count;

    if (stored_count > MAX_NETWORKS)
    {
        stored_count = MAX_NETWORKS;
    }

    wifi_ap_record_t *records = nullptr;

    if (discovered_count > 0)
    {
        records =
            new wifi_ap_record_t[discovered_count];

        if (records == nullptr)
        {
            LOG_ERROR(
                WIFI,
                "Failed to allocate scan records");

            scanning = false;
            return false;
        }

        uint16_t record_count =
            discovered_count;

        err =
            esp_wifi_scan_get_ap_records(
                &record_count,
                records);

        if (err != ESP_OK)
        {
            LOG_ERROR(
                WIFI,
                "Failed to get Wi-Fi records: %s",
                esp_err_to_name(err));

            delete[] records;

            scanning = false;
            return false;
        }

        stored_count = record_count;

        if (stored_count > MAX_NETWORKS)
        {
            stored_count = MAX_NETWORKS;
        }

        /*
         * Replace the previous scan results
         * only after a successful scan.
         */
        memset(
            networks,
            0,
            sizeof(networks));

        network_count = 0;

        for (uint16_t i = 0;
             i < stored_count;
             i++)
        {
            const wifi_ap_record_t &record =
                records[i];

            WiFiNetwork &network =
                networks[network_count];

            memset(
                network.ssid,
                0,
                sizeof(network.ssid));

            /*
             * ESP-IDF provides an SSID buffer
             * of 33 bytes including room for the
             * null terminator.
             */
            memcpy(
                network.ssid,
                record.ssid,
                sizeof(network.ssid) - 1);

            memcpy(
                network.bssid,
                record.bssid,
                sizeof(network.bssid));

            network.rssi =
                record.rssi;

            network.channel =
                record.primary;

            network.hidden =
                (record.ssid[0] == '\0');

            network.security =
                convert_security(
                    record.authmode);

            LOG_INFO(
                WIFI,
                "Network %u: SSID=\"%s\" RSSI=%d Channel=%u",
                network_count,
                network.hidden
                    ? "<hidden>"
                    : network.ssid,
                network.rssi,
                network.channel);

            network_count++;
        }

        delete[] records;
    }
    else
    {
        memset(
            networks,
            0,
            sizeof(networks));

        network_count = 0;
    }

    scanning = false;

    LOG_INFO(
        WIFI,
        "Wi-Fi scan complete: %u networks stored",
        network_count);

    return true;
}

bool WiFiScanner::is_scanning()
{
    return scanning;
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