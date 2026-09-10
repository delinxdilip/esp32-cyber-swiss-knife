#pragma once

#include <cstdint>

// ------------------------------------------------------------
// UI MODEL
// ------------------------------------------------------------
//
// This structure contains the information required by the UI.
//
// During UI development this uses dummy values.
// Later these fields will be populated from the real
// firmware managers.
//
// ------------------------------------------------------------

struct UIModel
{
    // --------------------------------------------------------
    // SYSTEM
    // --------------------------------------------------------

    uint8_t temperature_celsius;

    uint32_t uptime_seconds;

    // --------------------------------------------------------
    // NETWORK
    // --------------------------------------------------------

    bool wifi_connected;

    uint8_t wifi_signal_percent;

    bool ap_running;

    uint8_t ap_clients;

    // --------------------------------------------------------
    // BLUETOOTH
    // --------------------------------------------------------

    bool bluetooth_enabled;
};