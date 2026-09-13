#pragma once

#include <stdint.h>

#include "ui/display/display_types.h"

// ============================================================
// AP CONFIGURATION
// ============================================================

struct APConfig
{
    char ssid[33];
    char password[65];

    bool enabled;

    uint8_t max_connections;
    uint8_t channel;
};

// ============================================================
// DEVICE CONFIGURATION
// ============================================================

struct DeviceConfig
{
    char device_name[33];

    char web_ui_title[65];

    bool ap_identification;
    bool device_discovery;

    char bluetooth_name[33];

    uint32_t tft_color_theme;
};

// ============================================================
// HARDWARE CONFIGURATION
// ============================================================

struct HardwareConfig
{
    bool onboard_rgb_led_enabled;

    uint8_t onboard_rgb_pin;
};

// ============================================================
// LOG CONFIGURATION
// ============================================================

struct LoggingConfig
{
    bool system_auto_clear;
    bool activity_auto_clear;
};

// ============================================================
// CONFIG MANAGER
// ============================================================

class ConfigManager
{
public:
    static bool init();

    static bool load();

    static bool save();

    // --------------------------------------------------------
    // AP CONFIGURATION
    // --------------------------------------------------------

    static const APConfig &get_ap_config();

    static bool validate(
        const APConfig &config);

    static bool set_ap_config(
        const APConfig &config);

    // --------------------------------------------------------
    // TFT CONFIGURATION
    // --------------------------------------------------------

    static const TFTConfig &get_tft_config();

    static bool validate(
        const TFTConfig &config);

    static bool set_tft_config(
        const TFTConfig &config);

    // --------------------------------------------------------
    // DEVICE CONFIGURATION
    // --------------------------------------------------------

    static const DeviceConfig &get_device_config();

    static bool validate(
        const DeviceConfig &config);

    static bool set_device_config(
        const DeviceConfig &config);

    // --------------------------------------------------------
    // HARDWARE CONFIGURATION
    // --------------------------------------------------------

    static const HardwareConfig &get_hardware_config();

    static bool validate(
        const HardwareConfig &config);

    static bool set_hardware_config(
        const HardwareConfig &config);

    // --------------------------------------------------------
    // LOG CONFIGURATION
    // --------------------------------------------------------

    static const LoggingConfig &get_logging_config();

    static bool set_logging_config(
        const LoggingConfig &config);

private:
    static bool save_ap_config();
    static bool save_tft_config();
    static bool save_device_config();
    static bool save_hardware_config();
    static bool save_logging_config();

    static APConfig ap_config;
    static TFTConfig tft_config;
    static DeviceConfig device_config;
    static HardwareConfig hardware_config;
    static LoggingConfig logging_config;

    static void set_defaults();
};