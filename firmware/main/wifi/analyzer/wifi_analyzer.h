#pragma once

#include <stdint.h>

enum class WiFiSignalLevel
{
    EXCELLENT,
    GOOD,
    FAIR,
    WEAK,
    VERY_WEAK,
    UNKNOWN
};

class WiFiAnalyzer
{
public:
    static bool analyze();

    static uint16_t get_network_count();

    static int8_t get_strongest_rssi();
    static int8_t get_weakest_rssi();

    static uint16_t get_open_network_count();
    static uint16_t get_hidden_network_count();

    static uint16_t get_channel_count(uint8_t channel);

    static WiFiSignalLevel get_signal_level(int8_t rssi);
};