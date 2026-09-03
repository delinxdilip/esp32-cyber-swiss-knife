#include "wifi_analyzer.h"

#include "core/logging/logger.h"
#include "wifi/scanner/wifi_scanner.h"

namespace
{
    uint16_t network_count = 0;

    int8_t strongest_rssi = -127;
    int8_t weakest_rssi = 0;

    uint16_t open_network_count = 0;
    uint16_t hidden_network_count = 0;

    uint16_t channel_counts[14] = {};

    const char *signal_level_to_string(
        WiFiSignalLevel level)
    {
        switch (level)
        {
            case WiFiSignalLevel::EXCELLENT:
                return "EXCELLENT";

            case WiFiSignalLevel::GOOD:
                return "GOOD";

            case WiFiSignalLevel::FAIR:
                return "FAIR";

            case WiFiSignalLevel::WEAK:
                return "WEAK";

            case WiFiSignalLevel::VERY_WEAK:
                return "VERY_WEAK";

            default:
                return "UNKNOWN";
        }
    }
}

bool WiFiAnalyzer::analyze()
{
    LOG_INFO(
        WIFI,
        "Starting Wi-Fi analysis");

    // ----------------------------------------
    // Reset previous analysis
    // ----------------------------------------

    network_count = 0;
    strongest_rssi = -127;
    weakest_rssi = 0;
    open_network_count = 0;
    hidden_network_count = 0;

    for (uint8_t channel = 0; channel < 14; channel++)
    {
        channel_counts[channel] = 0;
    }

    // ----------------------------------------
    // Get scanner results
    // ----------------------------------------

    uint16_t count =
        WiFiScanner::get_count();

    if (count == 0)
    {
        LOG_WARN(
            WIFI,
            "No Wi-Fi networks available for analysis");

        return true;
    }

    // ----------------------------------------
    // Analyze networks
    // ----------------------------------------

    for (uint16_t i = 0; i < count; i++)
    {
        const WiFiNetwork *network =
            WiFiScanner::get_network(i);

        if (network == nullptr)
        {
            continue;
        }

        network_count++;

        // ----------------------------------------
        // Signal classification
        // ----------------------------------------

        LOG_INFO(
            WIFI,
            "Network: %s | RSSI: %d dBm | Signal: %s",
            network->ssid,
            network->rssi,
            signal_level_to_string(
                get_signal_level(network->rssi)));

        // ----------------------------------------
        // RSSI
        // ----------------------------------------

        if (network->rssi > strongest_rssi)
        {
            strongest_rssi =
                network->rssi;
        }

        if (network->rssi < weakest_rssi)
        {
            weakest_rssi =
                network->rssi;
        }

        // ----------------------------------------
        // Open networks
        // ----------------------------------------

        if (network->security ==
            WiFiSecurity::OPEN)
        {
            open_network_count++;
        }

        // ----------------------------------------
        // Hidden networks
        // ----------------------------------------

        if (network->hidden)
        {
            hidden_network_count++;
        }

        // ----------------------------------------
        // Channel distribution
        // ----------------------------------------

        if (network->channel < 14)
        {
            channel_counts[
                network->channel]++;
        }
    }

    // ----------------------------------------
    // Log analysis
    // ----------------------------------------

    LOG_INFO(
        WIFI,
        "Analysis complete");

    LOG_INFO(
        WIFI,
        "Networks analyzed: %u",
        network_count);

    LOG_INFO(
        WIFI,
        "Strongest RSSI: %d dBm",
        strongest_rssi);

    LOG_INFO(
        WIFI,
        "Weakest RSSI: %d dBm",
        weakest_rssi);

    LOG_INFO(
        WIFI,
        "Open networks: %u",
        open_network_count);

    LOG_INFO(
        WIFI,
        "Hidden networks: %u",
        hidden_network_count);

    for (uint8_t channel = 1; channel <= 13; channel++)
    {
        if (channel_counts[channel] > 0)
        {
            LOG_INFO(
                WIFI,
                "Channel %u: %u network(s)",
                channel,
                channel_counts[channel]);
        }
    }

    return true;
}

uint16_t WiFiAnalyzer::get_network_count()
{
    return network_count;
}

int8_t WiFiAnalyzer::get_strongest_rssi()
{
    return strongest_rssi;
}

int8_t WiFiAnalyzer::get_weakest_rssi()
{
    return weakest_rssi;
}

uint16_t WiFiAnalyzer::get_open_network_count()
{
    return open_network_count;
}

uint16_t WiFiAnalyzer::get_hidden_network_count()
{
    return hidden_network_count;
}

uint16_t WiFiAnalyzer::get_channel_count(
    uint8_t channel)
{
    if (channel >= 14)
    {
        return 0;
    }

    return channel_counts[channel];
}

WiFiSignalLevel WiFiAnalyzer::get_signal_level(
    int8_t rssi)
{
    if (rssi >= -50)
    {
        return WiFiSignalLevel::EXCELLENT;
    }

    if (rssi >= -60)
    {
        return WiFiSignalLevel::GOOD;
    }

    if (rssi >= -70)
    {
        return WiFiSignalLevel::FAIR;
    }

    if (rssi >= -80)
    {
        return WiFiSignalLevel::WEAK;
    }

    return WiFiSignalLevel::VERY_WEAK;
}