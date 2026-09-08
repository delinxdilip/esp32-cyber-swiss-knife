#pragma once

#include <stdint.h>

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
// TFT CONFIGURATION
// ============================================================

enum class TFTController : uint8_t
{
    GC9A01 = 0
};

enum class TFTShape : uint8_t
{
    ROUND = 0
};

enum class TFTRotation : uint8_t
{
    ROTATION_0   = 0,
    ROTATION_90  = 1,
    ROTATION_180 = 2,
    ROTATION_270 = 3
};

struct TFTConfig
{
    bool enabled;

    TFTController controller;
    TFTShape shape;

    uint16_t width;
    uint16_t height;

    TFTRotation rotation;

    bool touch;

    uint32_t color;
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

private:
    static bool save_ap_config();
    static bool save_tft_config();
    static bool save_device_config();
    static bool save_hardware_config();

    static APConfig ap_config;
    static TFTConfig tft_config;
    static DeviceConfig device_config;
    static HardwareConfig hardware_config;

    static void set_defaults();
};