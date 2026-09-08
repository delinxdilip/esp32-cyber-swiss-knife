#include "config_manager.h"

#include <cstring>

#include "nvs.h"

#include "core/logging/logger.h"

namespace
{
    // ========================================================
    // NVS
    // ========================================================

    constexpr const char *NVS_NAMESPACE =
        "config";

    // ========================================================
    // AP KEYS
    // ========================================================

    constexpr const char *KEY_AP_ENABLED =
        "ap_enabled";

    constexpr const char *KEY_AP_SSID =
        "ap_ssid";

    constexpr const char *KEY_AP_PASSWORD =
        "ap_password";

    constexpr const char *KEY_AP_MAX_CONN =
        "ap_max_conn";

    constexpr const char *KEY_AP_CHANNEL =
        "ap_channel";

    // ========================================================
    // TFT KEYS
    // ========================================================

    constexpr const char *KEY_TFT_ENABLED =
        "tft_enabled";

    constexpr const char *KEY_TFT_CONTROLLER =
        "tft_controller";

    constexpr const char *KEY_TFT_SHAPE =
        "tft_shape";

    constexpr const char *KEY_TFT_WIDTH =
        "tft_width";

    constexpr const char *KEY_TFT_HEIGHT =
        "tft_height";

    constexpr const char *KEY_TFT_ROTATION =
        "tft_rotation";

    constexpr const char *KEY_TFT_TOUCH =
        "tft_touch";

    constexpr const char *KEY_TFT_COLOR =
        "tft_color";

    // ========================================================
    // DEVICE KEYS
    // ========================================================

    constexpr const char *KEY_DEVICE_NAME =
        "device_name";

    constexpr const char *KEY_WEB_UI_TITLE =
        "web_ui_title";

    constexpr const char *KEY_AP_IDENTIFICATION =
        "ap_identification";

    constexpr const char *KEY_DEVICE_DISCOVERY =
        "device_discovery";

    constexpr const char *KEY_BLUETOOTH_NAME =
        "bluetooth_name";

    constexpr const char *KEY_TFT_COLOR_THEME =
        "tft_color_theme";

    // ========================================================
    // HARDWARE KEYS
    // ========================================================

    constexpr const char *KEY_RGB_LED_ENABLED =
        "rgb_led_enabled";

    constexpr const char *KEY_RGB_LED_PIN =
        "rgb_led_pin";

    // ========================================================
    // LOG KEYS
    // ========================================================

    constexpr const char *KEY_LOG_SYSTEM_AUTO_CLEAR =
        "log_system_auto_clear";

    constexpr const char *KEY_LOG_ACTIVITY_AUTO_CLEAR =
        "log_activity_auto_clear";
}


// ============================================================
// STATIC CONFIGURATION INSTANCES
// ============================================================

APConfig ConfigManager::ap_config = {};

TFTConfig ConfigManager::tft_config = {};

DeviceConfig ConfigManager::device_config = {};

HardwareConfig ConfigManager::hardware_config = {};

LoggingConfig ConfigManager::logging_config = {};


// ============================================================
// INITIALIZATION
// ============================================================

bool ConfigManager::init()
{
    return load();
}


// ============================================================
// DEFAULT CONFIGURATION
// ============================================================

void ConfigManager::set_defaults()
{
    // --------------------------------------------------------
    // AP DEFAULTS
    // --------------------------------------------------------

    std::memset(
        &ap_config,
        0,
        sizeof(ap_config));

    std::strncpy(
        ap_config.ssid,
        "CyberSwissKnife",
        sizeof(ap_config.ssid) - 1);

    std::strncpy(
        ap_config.password,
        "csk-local-2026",
        sizeof(ap_config.password) - 1);

    ap_config.enabled = true;
    ap_config.max_connections = 4;
    ap_config.channel = 1;

    // --------------------------------------------------------
    // TFT DEFAULTS
    // --------------------------------------------------------

    std::memset(
        &tft_config,
        0,
        sizeof(tft_config));

    tft_config.enabled = true;

    tft_config.controller =
        TFTController::GC9A01;

    tft_config.shape =
        TFTShape::ROUND;

    tft_config.width = 240;
    tft_config.height = 240;

    tft_config.rotation =
        TFTRotation::ROTATION_0;

    tft_config.touch = false;

    tft_config.color = 0xFFFFFF;

    // --------------------------------------------------------
    // DEVICE DEFAULTS
    // --------------------------------------------------------

    std::memset(
        &device_config,
        0,
        sizeof(device_config));

    std::strncpy(
        device_config.device_name,
        "CyberSwissKnife",
        sizeof(device_config.device_name) - 1);

    std::strncpy(
        device_config.web_ui_title,
        "ESP32 Cyber Swiss Knife",
        sizeof(device_config.web_ui_title) - 1);

    device_config.ap_identification = true;

    device_config.device_discovery = true;

    std::strncpy(
        device_config.bluetooth_name,
        "CyberSwissKnife",
        sizeof(device_config.bluetooth_name) - 1);

    device_config.tft_color_theme = 0xFFFFFF;

    // --------------------------------------------------------
    // HARDWARE DEFAULTS
    // --------------------------------------------------------

    std::memset(
        &hardware_config,
        0,
        sizeof(hardware_config));

    hardware_config.onboard_rgb_led_enabled = true;

    hardware_config.onboard_rgb_pin = 48;

    // --------------------------------------------------------
    // LOG DEFAULTS
    // --------------------------------------------------------

    std::memset(
        &logging_config,
        0,
        sizeof(logging_config));

    logging_config.system_auto_clear = true;
    logging_config.activity_auto_clear = true;
}


// ============================================================
// AP VALIDATION
// ============================================================

bool ConfigManager::validate(
    const APConfig &config)
{
    const size_t ssid_length =
        std::strlen(config.ssid);

    const size_t password_length =
        std::strlen(config.password);

    if (ssid_length == 0 ||
        ssid_length > 32)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid AP SSID length: %u",
            static_cast<unsigned>(
                ssid_length));

        return false;
    }

    if (password_length < 8 ||
        password_length > 64)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid AP password length");

        return false;
    }

    if (config.max_connections < 1 ||
        config.max_connections > 10)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid AP max connections: %u",
            static_cast<unsigned>(
                config.max_connections));

        return false;
    }

    if (config.channel < 1 ||
        config.channel > 13)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid AP channel: %u",
            static_cast<unsigned>(
                config.channel));

        return false;
    }

    return true;
}


// ============================================================
// TFT VALIDATION
// ============================================================

bool ConfigManager::validate(
    const TFTConfig &config)
{
    switch (config.controller)
    {
        case TFTController::GC9A01:
            break;

        default:
            LOG_WARN(
                ACTIVITY,
                SYSTEM,
                "Invalid TFT controller");

            return false;
    }

    switch (config.shape)
    {
        case TFTShape::ROUND:
            break;

        default:
            LOG_WARN(
                ACTIVITY,
                SYSTEM,
                "Invalid TFT shape");

            return false;
    }

    if (config.width == 0 ||
        config.height == 0)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid TFT resolution: %ux%u",
            static_cast<unsigned>(
                config.width),
            static_cast<unsigned>(
                config.height));

        return false;
    }

    if (config.controller ==
            TFTController::GC9A01 &&
        config.shape ==
            TFTShape::ROUND)
    {
        if (config.width != 240 ||
            config.height != 240)
        {
            LOG_WARN(
                ACTIVITY,
                SYSTEM,
                "Invalid GC9A01 round display resolution: %ux%u",
                static_cast<unsigned>(
                    config.width),
                static_cast<unsigned>(
                    config.height));

            return false;
        }
    }

    switch (config.rotation)
    {
        case TFTRotation::ROTATION_0:
        case TFTRotation::ROTATION_90:
        case TFTRotation::ROTATION_180:
        case TFTRotation::ROTATION_270:
            break;

        default:
            LOG_WARN(
                ACTIVITY,
                SYSTEM,
                "Invalid TFT rotation");

            return false;
    }

    return true;
}


// ============================================================
// DEVICE VALIDATION
// ============================================================

bool ConfigManager::validate(
    const DeviceConfig &config)
{
    const size_t device_name_length =
        std::strlen(config.device_name);

    const size_t web_ui_title_length =
        std::strlen(config.web_ui_title);

    const size_t bluetooth_name_length =
        std::strlen(config.bluetooth_name);

    if (device_name_length == 0 ||
        device_name_length > 32)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid device name length: %u",
            static_cast<unsigned>(
                device_name_length));

        return false;
    }

    if (web_ui_title_length == 0 ||
        web_ui_title_length > 64)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid Web UI title length: %u",
            static_cast<unsigned>(
                web_ui_title_length));

        return false;
    }

    if (bluetooth_name_length == 0 ||
        bluetooth_name_length > 32)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid Bluetooth name length: %u",
            static_cast<unsigned>(
                bluetooth_name_length));

        return false;
    }

    return true;
}


// ============================================================
// HARDWARE VALIDATION
// ============================================================

bool ConfigManager::validate(
    const HardwareConfig &config)
{
    if (config.onboard_rgb_pin > 48)
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Invalid onboard RGB LED GPIO: %u",
            static_cast<unsigned>(
                config.onboard_rgb_pin));

        return false;
    }

    return true;
}


// ============================================================
// LOAD CONFIGURATION
// ============================================================

bool ConfigManager::load()
{
    nvs_handle_t handle;

    esp_err_t err =
        nvs_open(
            NVS_NAMESPACE,
            NVS_READWRITE,
            &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to open config NVS: %s",
            esp_err_to_name(err));

        return false;
    }

    bool needs_save = false;

    // ========================================================
    // AP CONFIGURATION
    // ========================================================

    bool ap_loaded = true;

    size_t ssid_size =
        sizeof(ap_config.ssid);

    size_t password_size =
        sizeof(ap_config.password);

    err = nvs_get_str(
        handle,
        KEY_AP_SSID,
        ap_config.ssid,
        &ssid_size);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ap_loaded = false;
    }
    else if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to load AP SSID: %s",
            esp_err_to_name(err));

        nvs_close(handle);

        return false;
    }

    if (ap_loaded)
    {
        err = nvs_get_str(
            handle,
            KEY_AP_PASSWORD,
            ap_config.password,
            &password_size);

        if (err != ESP_OK)
        {
            ap_loaded = false;
        }
    }

    if (ap_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_AP_MAX_CONN,
            &ap_config.max_connections);

        if (err != ESP_OK)
        {
            ap_loaded = false;
        }
    }

    if (ap_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_AP_CHANNEL,
            &ap_config.channel);

        if (err != ESP_OK)
        {
            ap_loaded = false;
        }
    }

    if (ap_loaded)
    {
        uint8_t enabled = 1;

        err = nvs_get_u8(
            handle,
            KEY_AP_ENABLED,
            &enabled);

        if (err == ESP_OK)
        {
            ap_config.enabled =
                enabled != 0;
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ap_config.enabled = true;
            needs_save = true;
        }
        else
        {
            ap_loaded = false;
        }
    }

    if (!ap_loaded)
    {
        LOG_INFO(
            ACTIVITY,
            SYSTEM,
            "AP configuration missing or incomplete; using defaults");

        APConfig defaults = {};

        std::strncpy(
            defaults.ssid,
            "CyberSwissKnife",
            sizeof(defaults.ssid) - 1);

        std::strncpy(
            defaults.password,
            "csk-local-2026",
            sizeof(defaults.password) - 1);

        defaults.enabled = true;
        defaults.max_connections = 4;
        defaults.channel = 1;

        ap_config = defaults;

        needs_save = true;
    }
    else if (!validate(ap_config))
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Stored AP configuration is invalid; restoring AP defaults");

        APConfig defaults = {};

        std::strncpy(
            defaults.ssid,
            "CyberSwissKnife",
            sizeof(defaults.ssid) - 1);

        std::strncpy(
            defaults.password,
            "csk-local-2026",
            sizeof(defaults.password) - 1);

        defaults.enabled = true;
        defaults.max_connections = 4;
        defaults.channel = 1;

        ap_config = defaults;

        needs_save = true;
    }

    // ========================================================
    // TFT CONFIGURATION
    // ========================================================

    bool tft_loaded = true;

    uint8_t tft_enabled = 1;
    uint8_t tft_controller = 0;
    uint8_t tft_shape = 0;
    uint8_t tft_rotation = 0;
    uint8_t tft_touch = 0;

    err = nvs_get_u8(
        handle,
        KEY_TFT_ENABLED,
        &tft_enabled);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        tft_loaded = false;
    }
    else if (err != ESP_OK)
    {
        tft_loaded = false;
    }

    if (tft_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_TFT_CONTROLLER,
            &tft_controller);

        if (err != ESP_OK)
        {
            tft_loaded = false;
        }
    }

    if (tft_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_TFT_SHAPE,
            &tft_shape);

        if (err != ESP_OK)
        {
            tft_loaded = false;
        }
    }

    if (tft_loaded)
    {
        err = nvs_get_u16(
            handle,
            KEY_TFT_WIDTH,
            &tft_config.width);

        if (err != ESP_OK)
        {
            tft_loaded = false;
        }
    }

    if (tft_loaded)
    {
        err = nvs_get_u16(
            handle,
            KEY_TFT_HEIGHT,
            &tft_config.height);

        if (err != ESP_OK)
        {
            tft_loaded = false;
        }
    }

    if (tft_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_TFT_ROTATION,
            &tft_rotation);

        if (err != ESP_OK)
        {
            tft_loaded = false;
        }
    }

    if (tft_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_TFT_TOUCH,
            &tft_touch);

        if (err != ESP_OK)
        {
            tft_loaded = false;
        }
    }

    if (tft_loaded)
    {
        err = nvs_get_u32(
            handle,
            KEY_TFT_COLOR,
            &tft_config.color);

        if (err != ESP_OK)
        {
            tft_loaded = false;
        }
    }

    if (tft_loaded)
    {
        tft_config.enabled =
            tft_enabled != 0;

        tft_config.controller =
            static_cast<TFTController>(
                tft_controller);

        tft_config.shape =
            static_cast<TFTShape>(
                tft_shape);

        tft_config.rotation =
            static_cast<TFTRotation>(
                tft_rotation);

        tft_config.touch =
            tft_touch != 0;
    }

    if (!tft_loaded)
    {
        LOG_INFO(
            ACTIVITY,
            SYSTEM,
            "TFT configuration missing or incomplete; using defaults");

        tft_config.enabled = true;
        tft_config.controller = TFTController::GC9A01;
        tft_config.shape = TFTShape::ROUND;
        tft_config.width = 240;
        tft_config.height = 240;
        tft_config.rotation = TFTRotation::ROTATION_0;
        tft_config.touch = false;
        tft_config.color = 0xFFFFFF;

        needs_save = true;
    }
    else if (!validate(tft_config))
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Stored TFT configuration is invalid; restoring TFT defaults");

        tft_config.enabled = true;
        tft_config.controller = TFTController::GC9A01;
        tft_config.shape = TFTShape::ROUND;
        tft_config.width = 240;
        tft_config.height = 240;
        tft_config.rotation = TFTRotation::ROTATION_0;
        tft_config.touch = false;
        tft_config.color = 0xFFFFFF;

        needs_save = true;
    }

    // ========================================================
    // DEVICE CONFIGURATION
    // ========================================================

    bool device_loaded = true;

    size_t device_name_size =
        sizeof(device_config.device_name);

    size_t web_ui_title_size =
        sizeof(device_config.web_ui_title);

    size_t bluetooth_name_size =
        sizeof(device_config.bluetooth_name);

    err = nvs_get_str(
        handle,
        KEY_DEVICE_NAME,
        device_config.device_name,
        &device_name_size);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        device_loaded = false;
    }
    else if (err != ESP_OK)
    {
        device_loaded = false;
    }

    if (device_loaded)
    {
        err = nvs_get_str(
            handle,
            KEY_WEB_UI_TITLE,
            device_config.web_ui_title,
            &web_ui_title_size);

        if (err != ESP_OK)
        {
            device_loaded = false;
        }
    }

    if (device_loaded)
    {
        uint8_t value = 1;

        err = nvs_get_u8(
            handle,
            KEY_AP_IDENTIFICATION,
            &value);

        if (err == ESP_OK)
        {
            device_config.ap_identification =
                value != 0;
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            device_config.ap_identification = true;
            needs_save = true;
        }
        else
        {
            device_loaded = false;
        }
    }

    if (device_loaded)
    {
        uint8_t value = 1;

        err = nvs_get_u8(
            handle,
            KEY_DEVICE_DISCOVERY,
            &value);

        if (err == ESP_OK)
        {
            device_config.device_discovery =
                value != 0;
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            device_config.device_discovery = true;
            needs_save = true;
        }
        else
        {
            device_loaded = false;
        }
    }

    if (device_loaded)
    {
        err = nvs_get_str(
            handle,
            KEY_BLUETOOTH_NAME,
            device_config.bluetooth_name,
            &bluetooth_name_size);

        if (err != ESP_OK)
        {
            device_loaded = false;
        }
    }

    if (device_loaded)
    {
        err = nvs_get_u32(
            handle,
            KEY_TFT_COLOR_THEME,
            &device_config.tft_color_theme);

        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            device_config.tft_color_theme = 0xFFFFFF;
            needs_save = true;
        }
        else if (err != ESP_OK)
        {
            device_loaded = false;
        }
    }

    if (!device_loaded)
    {
        LOG_INFO(
            ACTIVITY,
            SYSTEM,
            "Device configuration missing or incomplete; using defaults");

        std::memset(
            &device_config,
            0,
            sizeof(device_config));

        std::strncpy(
            device_config.device_name,
            "CyberSwissKnife",
            sizeof(device_config.device_name) - 1);

        std::strncpy(
            device_config.web_ui_title,
            "ESP32 Cyber Swiss Knife",
            sizeof(device_config.web_ui_title) - 1);

        device_config.ap_identification = true;
        device_config.device_discovery = true;

        std::strncpy(
            device_config.bluetooth_name,
            "CyberSwissKnife",
            sizeof(device_config.bluetooth_name) - 1);

        device_config.tft_color_theme = 0xFFFFFF;

        needs_save = true;
    }
    else if (!validate(device_config))
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Stored device configuration is invalid; restoring device defaults");

        std::memset(
            &device_config,
            0,
            sizeof(device_config));

        std::strncpy(
            device_config.device_name,
            "CyberSwissKnife",
            sizeof(device_config.device_name) - 1);

        std::strncpy(
            device_config.web_ui_title,
            "ESP32 Cyber Swiss Knife",
            sizeof(device_config.web_ui_title) - 1);

        device_config.ap_identification = true;
        device_config.device_discovery = true;

        std::strncpy(
            device_config.bluetooth_name,
            "CyberSwissKnife",
            sizeof(device_config.bluetooth_name) - 1);

        device_config.tft_color_theme = 0xFFFFFF;

        needs_save = true;
    }

    // ========================================================
    // HARDWARE CONFIGURATION
    // ========================================================

    bool hardware_loaded = true;

    uint8_t rgb_led_enabled = 1;

    err = nvs_get_u8(
        handle,
        KEY_RGB_LED_ENABLED,
        &rgb_led_enabled);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        hardware_loaded = false;
    }
    else if (err != ESP_OK)
    {
        hardware_loaded = false;
    }

    if (hardware_loaded)
    {
        hardware_config.onboard_rgb_led_enabled =
            rgb_led_enabled != 0;
    }

    if (hardware_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_RGB_LED_PIN,
            &hardware_config.onboard_rgb_pin);

        if (err != ESP_OK)
        {
            hardware_loaded = false;
        }
    }

    if (!hardware_loaded)
    {
        LOG_INFO(
            ACTIVITY,
            SYSTEM,
            "Hardware configuration missing or incomplete; using defaults");

        std::memset(
            &hardware_config,
            0,
            sizeof(hardware_config));

        hardware_config.onboard_rgb_led_enabled = true;
        hardware_config.onboard_rgb_pin = 48;

        needs_save = true;
    }
    else if (!validate(hardware_config))
    {
        LOG_WARN(
            ACTIVITY,
            SYSTEM,
            "Stored hardware configuration is invalid; restoring hardware defaults");

        std::memset(
            &hardware_config,
            0,
            sizeof(hardware_config));

        hardware_config.onboard_rgb_led_enabled = true;
        hardware_config.onboard_rgb_pin = 48;

        needs_save = true;
    }

    // ========================================================
    // LOGGING CONFIGURATION
    // ========================================================

    bool logging_loaded = true;

    uint8_t system_auto_clear = 1;
    uint8_t activity_auto_clear = 1;

    err = nvs_get_u8(
        handle,
        KEY_LOG_SYSTEM_AUTO_CLEAR,
        &system_auto_clear);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        logging_loaded = false;
    }
    else if (err != ESP_OK)
    {
        logging_loaded = false;
    }

    if (logging_loaded)
    {
        err = nvs_get_u8(
            handle,
            KEY_LOG_ACTIVITY_AUTO_CLEAR,
            &activity_auto_clear);

        if (err != ESP_OK)
        {
            logging_loaded = false;
        }
    }

    if (logging_loaded)
    {
        logging_config.system_auto_clear =
            system_auto_clear != 0;

        logging_config.activity_auto_clear =
            activity_auto_clear != 0;
    }

    if (!logging_loaded)
    {
        LOG_INFO(
            ACTIVITY,
            SYSTEM,
            "Logging configuration missing or incomplete; using defaults");

        logging_config.system_auto_clear = true;
        logging_config.activity_auto_clear = true;

        needs_save = true;
    }

    nvs_close(handle);

    // ========================================================
    // SAVE MISSING / CORRECTED VALUES
    // ========================================================

    if (needs_save)
    {
        LOG_INFO(
            ACTIVITY,
            SYSTEM,
            "Configuration defaults or corrections need to be persisted");

        return save();
    }

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Configuration loaded successfully");

    return true;
}


// ============================================================
// SAVE CONFIGURATION
// ============================================================

bool ConfigManager::save()
{
    if (!validate(ap_config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Cannot save: invalid AP configuration");

        return false;
    }

    if (!validate(tft_config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Cannot save: invalid TFT configuration");

        return false;
    }

    if (!validate(device_config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Cannot save: invalid device configuration");

        return false;
    }

    if (!validate(hardware_config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Cannot save: invalid hardware configuration");

        return false;
    }

    nvs_handle_t handle;

    esp_err_t err =
        nvs_open(
            NVS_NAMESPACE,
            NVS_READWRITE,
            &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to open config NVS for save: %s",
            esp_err_to_name(err));

        return false;
    }

    // ========================================================
    // AP
    // ========================================================

    err = nvs_set_u8(
        handle,
        KEY_AP_ENABLED,
        ap_config.enabled ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_AP_SSID,
        ap_config.ssid);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_AP_PASSWORD,
        ap_config.password);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_AP_MAX_CONN,
        ap_config.max_connections);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_AP_CHANNEL,
        ap_config.channel);

    if (err != ESP_OK)
        goto error;

    // ========================================================
    // TFT
    // ========================================================

    err = nvs_set_u8(
        handle,
        KEY_TFT_ENABLED,
        tft_config.enabled ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_CONTROLLER,
        static_cast<uint8_t>(
            tft_config.controller));

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_SHAPE,
        static_cast<uint8_t>(
            tft_config.shape));

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u16(
        handle,
        KEY_TFT_WIDTH,
        tft_config.width);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u16(
        handle,
        KEY_TFT_HEIGHT,
        tft_config.height);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_ROTATION,
        static_cast<uint8_t>(
            tft_config.rotation));

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_TOUCH,
        tft_config.touch ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u32(
        handle,
        KEY_TFT_COLOR,
        tft_config.color);

    if (err != ESP_OK)
        goto error;

    // ========================================================
    // DEVICE
    // ========================================================

    err = nvs_set_str(
        handle,
        KEY_DEVICE_NAME,
        device_config.device_name);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_WEB_UI_TITLE,
        device_config.web_ui_title);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_AP_IDENTIFICATION,
        device_config.ap_identification ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_DEVICE_DISCOVERY,
        device_config.device_discovery ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_BLUETOOTH_NAME,
        device_config.bluetooth_name);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u32(
        handle,
        KEY_TFT_COLOR_THEME,
        device_config.tft_color_theme);

    if (err != ESP_OK)
        goto error;

    // ========================================================
    // HARDWARE
    // ========================================================

    err = nvs_set_u8(
        handle,
        KEY_RGB_LED_ENABLED,
        hardware_config.onboard_rgb_led_enabled ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_RGB_LED_PIN,
        hardware_config.onboard_rgb_pin);

    if (err != ESP_OK)
        goto error;

    // ========================================================
    // LOGGING
    // ========================================================

    err = nvs_set_u8(
        handle,
        KEY_LOG_SYSTEM_AUTO_CLEAR,
        logging_config.system_auto_clear ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_LOG_ACTIVITY_AUTO_CLEAR,
        logging_config.activity_auto_clear ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    // ========================================================
    // COMMIT
    // ========================================================

    err = nvs_commit(handle);

    if (err != ESP_OK)
        goto error;

    nvs_close(handle);

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Configuration saved");

    return true;

error:

    LOG_ERROR(
        ACTIVITY,
        SYSTEM,
        "Failed to save configuration: %s",
        esp_err_to_name(err));

    nvs_close(handle);

    return false;
}


// ============================================================
// AP GET / SET
// ============================================================

const APConfig &ConfigManager::get_ap_config()
{
    return ap_config;
}

bool ConfigManager::set_ap_config(
    const APConfig &config)
{
    if (!validate(config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Rejected invalid AP configuration");

        return false;
    }

    APConfig old_config = ap_config;

    ap_config = config;

    if (!save_ap_config())
    {
        ap_config = old_config;

        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to update AP configuration");

        return false;
    }

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "AP configuration updated");

    return true;
}


// ============================================================
// TFT GET / SET
// ============================================================

const TFTConfig &ConfigManager::get_tft_config()
{
    return tft_config;
}

bool ConfigManager::set_tft_config(
    const TFTConfig &config)
{
    if (!validate(config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Rejected invalid TFT configuration");

        return false;
    }

    TFTConfig old_config = tft_config;

    tft_config = config;

    if (!save_tft_config())
    {
        tft_config = old_config;

        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to update TFT configuration");

        return false;
    }

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "TFT configuration updated");

    return true;
}


// ============================================================
// DEVICE GET / SET
// ============================================================

const DeviceConfig &ConfigManager::get_device_config()
{
    return device_config;
}

bool ConfigManager::set_device_config(
    const DeviceConfig &config)
{
    if (!validate(config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Rejected invalid device configuration");

        return false;
    }

    DeviceConfig old_config = device_config;

    device_config = config;

    if (!save_device_config())
    {
        device_config = old_config;

        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to update device configuration");

        return false;
    }

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Device configuration updated");

    return true;
}


// ============================================================
// HARDWARE GET / SET
// ============================================================

const HardwareConfig &ConfigManager::get_hardware_config()
{
    return hardware_config;
}

bool ConfigManager::set_hardware_config(
    const HardwareConfig &config)
{
    if (!validate(config))
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Rejected invalid hardware configuration");

        return false;
    }

    HardwareConfig old_config = hardware_config;

    hardware_config = config;

    if (!save_hardware_config())
    {
        hardware_config = old_config;

        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to update hardware configuration");

        return false;
    }

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Hardware configuration updated");

    return true;
}


// ============================================================
// LOGGING GET / SET
// ============================================================

const LoggingConfig &ConfigManager::get_logging_config()
{
    return logging_config;
}

bool ConfigManager::set_logging_config(
    const LoggingConfig &config)
{
    LoggingConfig old_config =
        logging_config;

    logging_config = config;

    if (!save_logging_config())
    {
        logging_config = old_config;

        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to update logging configuration");

        return false;
    }

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Logging configuration updated");

    return true;
}


// ============================================================
// AP CONFIG SAVE
// ============================================================

bool ConfigManager::save_ap_config()
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(
        NVS_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to open NVS for AP config: %s",
            esp_err_to_name(err));

        return false;
    }

    err = nvs_set_u8(
        handle,
        KEY_AP_ENABLED,
        ap_config.enabled ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_AP_SSID,
        ap_config.ssid);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_AP_PASSWORD,
        ap_config.password);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_AP_CHANNEL,
        ap_config.channel);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_AP_MAX_CONN,
        ap_config.max_connections);

    if (err != ESP_OK)
        goto error;

    err = nvs_commit(handle);

    if (err != ESP_OK)
        goto error;

    nvs_close(handle);

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "AP configuration saved");

    return true;

error:

    LOG_ERROR(
        ACTIVITY,
        SYSTEM,
        "Failed to save AP configuration: %s",
        esp_err_to_name(err));

    nvs_close(handle);

    return false;
}


// ============================================================
// TFT CONFIG SAVE
// ============================================================

bool ConfigManager::save_tft_config()
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(
        NVS_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to open NVS for TFT config: %s",
            esp_err_to_name(err));

        return false;
    }

    err = nvs_set_u8(
        handle,
        KEY_TFT_ENABLED,
        tft_config.enabled ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_CONTROLLER,
        static_cast<uint8_t>(
            tft_config.controller));

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_SHAPE,
        static_cast<uint8_t>(
            tft_config.shape));

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u16(
        handle,
        KEY_TFT_WIDTH,
        tft_config.width);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u16(
        handle,
        KEY_TFT_HEIGHT,
        tft_config.height);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_ROTATION,
        static_cast<uint8_t>(
            tft_config.rotation));

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_TFT_TOUCH,
        tft_config.touch ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u32(
        handle,
        KEY_TFT_COLOR,
        tft_config.color);

    if (err != ESP_OK)
        goto error;

    err = nvs_commit(handle);

    if (err != ESP_OK)
        goto error;

    nvs_close(handle);

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "TFT configuration saved");

    return true;

error:

    LOG_ERROR(
        ACTIVITY,
        SYSTEM,
        "Failed to save TFT configuration: %s",
        esp_err_to_name(err));

    nvs_close(handle);

    return false;
}


// ============================================================
// DEVICE CONFIG SAVE
// ============================================================

bool ConfigManager::save_device_config()
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(
        NVS_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to open NVS for device config: %s",
            esp_err_to_name(err));

        return false;
    }

    err = nvs_set_str(
        handle,
        KEY_DEVICE_NAME,
        device_config.device_name);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_WEB_UI_TITLE,
        device_config.web_ui_title);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_AP_IDENTIFICATION,
        device_config.ap_identification ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_DEVICE_DISCOVERY,
        device_config.device_discovery ? 1 : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_str(
        handle,
        KEY_BLUETOOTH_NAME,
        device_config.bluetooth_name);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u32(
        handle,
        KEY_TFT_COLOR_THEME,
        device_config.tft_color_theme);

    if (err != ESP_OK)
        goto error;

    err = nvs_commit(handle);

    if (err != ESP_OK)
        goto error;

    nvs_close(handle);

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Device configuration saved");

    return true;

error:

    LOG_ERROR(
        ACTIVITY,
        SYSTEM,
        "Failed to save device configuration: %s",
        esp_err_to_name(err));

    nvs_close(handle);

    return false;
}


// ============================================================
// HARDWARE CONFIG SAVE
// ============================================================

bool ConfigManager::save_hardware_config()
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(
        NVS_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to open NVS for hardware config: %s",
            esp_err_to_name(err));

        return false;
    }

    err = nvs_set_u8(
        handle,
        KEY_RGB_LED_ENABLED,
        hardware_config.onboard_rgb_led_enabled
            ? 1
            : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_RGB_LED_PIN,
        hardware_config.onboard_rgb_pin);

    if (err != ESP_OK)
        goto error;

    err = nvs_commit(handle);

    if (err != ESP_OK)
        goto error;

    nvs_close(handle);

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Hardware configuration saved");

    return true;

error:

    LOG_ERROR(
        ACTIVITY,
        SYSTEM,
        "Failed to save hardware configuration: %s",
        esp_err_to_name(err));

    nvs_close(handle);

    return false;
}


// ============================================================
// LOGGING CONFIG SAVE
// ============================================================

bool ConfigManager::save_logging_config()
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(
        NVS_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            ACTIVITY,
            SYSTEM,
            "Failed to open NVS for logging config: %s",
            esp_err_to_name(err));

        return false;
    }

    err = nvs_set_u8(
        handle,
        KEY_LOG_SYSTEM_AUTO_CLEAR,
        logging_config.system_auto_clear
            ? 1
            : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_set_u8(
        handle,
        KEY_LOG_ACTIVITY_AUTO_CLEAR,
        logging_config.activity_auto_clear
            ? 1
            : 0);

    if (err != ESP_OK)
        goto error;

    err = nvs_commit(handle);

    if (err != ESP_OK)
        goto error;

    nvs_close(handle);

    LOG_INFO(
        ACTIVITY,
        SYSTEM,
        "Logging configuration saved");

    return true;

error:

    LOG_ERROR(
        ACTIVITY,
        SYSTEM,
        "Failed to save logging configuration: %s",
        esp_err_to_name(err));

    nvs_close(handle);

    return false;
}
