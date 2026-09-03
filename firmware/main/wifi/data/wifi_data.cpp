#include "wifi_data.h"

bool WiFiData::get_snapshot(
    WiFiSnapshot &snapshot)
{
    snapshot.network_count =
        WiFiScanner::get_count();

    snapshot.strongest_rssi =
        WiFiAnalyzer::get_strongest_rssi();

    snapshot.weakest_rssi =
        WiFiAnalyzer::get_weakest_rssi();

    snapshot.open_network_count =
        WiFiAnalyzer::get_open_network_count();

    snapshot.hidden_network_count =
        WiFiAnalyzer::get_hidden_network_count();

    for (uint8_t channel = 0;
         channel < 14;
         channel++)
    {
        snapshot.channel_counts[channel] =
            WiFiAnalyzer::get_channel_count(channel);
    }

    return true;
}

uint16_t WiFiData::get_network_count()
{
    return WiFiScanner::get_count();
}

const WiFiNetwork *WiFiData::get_network(
    uint16_t index)
{
    return WiFiScanner::get_network(index);
}

int8_t WiFiData::get_strongest_rssi()
{
    return WiFiAnalyzer::get_strongest_rssi();
}

int8_t WiFiData::get_weakest_rssi()
{
    return WiFiAnalyzer::get_weakest_rssi();
}

uint16_t WiFiData::get_open_network_count()
{
    return WiFiAnalyzer::get_open_network_count();
}

uint16_t WiFiData::get_hidden_network_count()
{
    return WiFiAnalyzer::get_hidden_network_count();
}

uint16_t WiFiData::get_channel_count(
    uint8_t channel)
{
    return WiFiAnalyzer::get_channel_count(channel);
}

WiFiSignalLevel WiFiData::get_signal_level(
    int8_t rssi)
{
    return WiFiAnalyzer::get_signal_level(rssi);
}