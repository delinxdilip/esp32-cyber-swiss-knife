#pragma once

#include <cstdint>
#include "esp_err.h"

namespace WiFiStatus
{
    struct Info
    {
        bool enabled;
        bool connected;

        char ssid[33];
        char ip_address[16];

        int8_t rssi;
        uint8_t channel;

        char security[16];
    };

    /**
     * Initialize the status data structure.
     */
    void init();

    /**
     * Read the current Wi-Fi station status.
     *
     * This reads information about the network that the ESP32
     * is currently connected to.
     */
    esp_err_t update();

    /**
     * Get the most recently collected Wi-Fi status.
     */
    const Info &get_info();

    /**
     * Check whether the station is currently connected.
     */
    bool is_connected();
}