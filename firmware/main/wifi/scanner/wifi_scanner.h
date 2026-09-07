#pragma once

#include <stdint.h>

enum class WiFiSecurity
{
    OPEN,
    WEP,
    WPA,
    WPA2,
    WPA_WPA2,
    WPA3,
    WPA2_WPA3,
    UNKNOWN
};

struct WiFiNetwork
{
    char ssid[33];
    uint8_t bssid[6];
    int8_t rssi;
    uint8_t channel;
    bool hidden;
    WiFiSecurity security;
};

class WiFiScanner
{
public:
    static bool scan();

    static bool is_scanning();

    static uint16_t get_count();

    static const WiFiNetwork *get_network(
        uint16_t index);
};