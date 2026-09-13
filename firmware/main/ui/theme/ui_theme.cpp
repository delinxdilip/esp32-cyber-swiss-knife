#include "ui_theme.h"

#include "core/config/config_manager.h"

// ------------------------------------------------------------
// UI THEME STATE
// ------------------------------------------------------------

namespace
{
    constexpr uint32_t UI_BACKGROUND =
        0x000000;
}

// ------------------------------------------------------------
// STATIC STATE
// ------------------------------------------------------------

uint32_t UITheme::theme_color =
    0xFFFFFF;

// ------------------------------------------------------------
// INITIALIZE
// ------------------------------------------------------------

void UITheme::init()
{
    const DeviceConfig &config =
        ConfigManager::get_device_config();

    theme_color =
        config.tft_color_theme;
}

// ------------------------------------------------------------
// BACKGROUND
// ------------------------------------------------------------

uint32_t UITheme::background()
{
    return UI_BACKGROUND;
}

// ------------------------------------------------------------
// PRIMARY
// ------------------------------------------------------------

uint32_t UITheme::primary()
{
    return theme_color;
}

// ------------------------------------------------------------
// SECONDARY
// ------------------------------------------------------------

uint32_t UITheme::secondary()
{
    return scale_color(
        theme_color,
        70);
}

// ------------------------------------------------------------
// DISABLED
// ------------------------------------------------------------

uint32_t UITheme::disabled()
{
    return scale_color(
        theme_color,
        35);
}

// ------------------------------------------------------------
// SELECTION
// ------------------------------------------------------------

uint32_t UITheme::selection()
{
    return scale_color(
        theme_color,
        50);
}

// ------------------------------------------------------------
// BORDER
// ------------------------------------------------------------

uint32_t UITheme::border()
{
    return scale_color(
        theme_color,
        45);
}

// ------------------------------------------------------------
// SCALE COLOR
// ------------------------------------------------------------

uint32_t UITheme::scale_color(
    uint32_t color,
    uint8_t percentage)
{
    uint8_t red =
        static_cast<uint8_t>(
            (color >> 16) & 0xFF);

    uint8_t green =
        static_cast<uint8_t>(
            (color >> 8) & 0xFF);

    uint8_t blue =
        static_cast<uint8_t>(
            color & 0xFF);

    red =
        static_cast<uint8_t>(
            (red * percentage) / 100);

    green =
        static_cast<uint8_t>(
            (green * percentage) / 100);

    blue =
        static_cast<uint8_t>(
            (blue * percentage) / 100);

    return
        (static_cast<uint32_t>(red) << 16) |
        (static_cast<uint32_t>(green) << 8) |
        static_cast<uint32_t>(blue);
}