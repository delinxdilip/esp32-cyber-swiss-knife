#pragma once

#include <stdint.h>

#include "wifi/scanner/wifi_scanner.h"
#include "wifi/analyzer/wifi_analyzer.h"

struct WiFiSnapshot
{
    uint16_t network_count;

    int8_t strongest_rssi;
    int8_t weakest_rssi;

    uint16_t open_network_count;
    uint16_t hidden_network_count;

    uint16_t channel_counts[14];
};

class WiFiData
{
public:
    static bool get_snapshot(
        WiFiSnapshot &snapshot);

    static uint16_t get_network_count();

    static const WiFiNetwork *get_network(
        uint16_t index);

    static int8_t get_strongest_rssi();

    static int8_t get_weakest_rssi();

    static uint16_t get_open_network_count();

    static uint16_t get_hidden_network_count();

    static uint16_t get_channel_count(
        uint8_t channel);

    static WiFiSignalLevel get_signal_level(
        int8_t rssi);
};