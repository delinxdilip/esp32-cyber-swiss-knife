#include "home_screen.h"

#include "ui/display/display_manager.h"
#include "ui/text/text_renderer.h"
#include "ui/theme/ui_theme.h"

// ------------------------------------------------------------
// HOME SCREEN CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr int16_t SCREEN_WIDTH = 240;
    constexpr int16_t SCREEN_HEIGHT = 240;

    constexpr int16_t HEADER_Y = 18;

    constexpr int16_t TIME_Y = 48;

    constexpr int16_t TEMP_Y = 92;
    constexpr int16_t WIFI_Y = 120;
    constexpr int16_t BLUETOOTH_Y = 148;
    constexpr int16_t AP_Y = 176;
    constexpr int16_t UPTIME_Y = 204;

    constexpr int16_t FOOTER_Y = 226;
}

// ------------------------------------------------------------
// CENTER TEXT
// ------------------------------------------------------------

namespace
{
    void draw_centered(
        const char *text,
        int16_t y,
        TextSize size,
        uint32_t color)
    {
        const uint16_t width =
            TextRenderer::get_width(
                text,
                size);

        const int16_t x =
            static_cast<int16_t>(
                (SCREEN_WIDTH - width) / 2);

        TextRenderer::draw(
            text,
            x,
            y,
            size,
            color);
    }
}

// ------------------------------------------------------------
// RENDER HOME SCREEN
// ------------------------------------------------------------

void HomeScreen::render(
    const UIModel &model)
{
    if (!DisplayManager::is_initialized() ||
        !DisplayManager::has_driver())
    {
        return;
    }

    // --------------------------------------------------------
    // COLORS
    // --------------------------------------------------------

    const uint32_t background =
        UITheme::background();

    const uint32_t primary =
        UITheme::primary();

    const uint32_t secondary =
        UITheme::secondary();

    const uint32_t disabled =
        UITheme::disabled();

    // --------------------------------------------------------
    // BACKGROUND
    // --------------------------------------------------------

    DisplayManager::clear(
        background);

    // --------------------------------------------------------
    // HEADER
    // --------------------------------------------------------

    draw_centered(
        "CYBERSWISS",
        HEADER_Y,
        TextSize::SMALL,
        primary);

    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    draw_centered(
        "23:47",
        TIME_Y,
        TextSize::LARGE,
        primary);

    // --------------------------------------------------------
    // TEMPERATURE
    // --------------------------------------------------------

    TextRenderer::draw(
        "TEMP",
        32,
        TEMP_Y,
        TextSize::SMALL,
        secondary);

    TextRenderer::draw(
        "42C",
        152,
        TEMP_Y,
        TextSize::SMALL,
        primary);

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    TextRenderer::draw(
        "WIFI",
        32,
        WIFI_Y,
        TextSize::SMALL,
        secondary);

    TextRenderer::draw(
        "100%",
        128,
        WIFI_Y,
        TextSize::SMALL,
        primary);

    DisplayManager::fill_circle(
        202,
        WIFI_Y + 5,
        4,
        model.wifi_connected
            ? primary
            : disabled);

    // --------------------------------------------------------
    // BLUETOOTH
    // --------------------------------------------------------

    TextRenderer::draw(
        "BLUETOOTH",
        32,
        BLUETOOTH_Y,
        TextSize::SMALL,
        secondary);

    TextRenderer::draw(
        model.bluetooth_enabled
            ? "ON"
            : "OFF",
        166,
        BLUETOOTH_Y,
        TextSize::SMALL,
        model.bluetooth_enabled
            ? primary
            : disabled);

    // --------------------------------------------------------
    // ACCESS POINT
    // --------------------------------------------------------

    TextRenderer::draw(
        "AP",
        32,
        AP_Y,
        TextSize::SMALL,
        secondary);

    TextRenderer::draw(
        model.ap_running
            ? "ON"
            : "OFF",
        82,
        AP_Y,
        TextSize::SMALL,
        model.ap_running
            ? primary
            : disabled);

    TextRenderer::draw(
        "03",
        166,
        AP_Y,
        TextSize::SMALL,
        primary);

    // --------------------------------------------------------
    // UPTIME
    // --------------------------------------------------------

    TextRenderer::draw(
        "UPTIME",
        32,
        UPTIME_Y,
        TextSize::SMALL,
        secondary);

    TextRenderer::draw(
        "00:12:34",
        128,
        UPTIME_Y,
        TextSize::SMALL,
        primary);

    // --------------------------------------------------------
    // FOOTER
    // --------------------------------------------------------

    draw_centered(
        "HOME 01/05",
        FOOTER_Y,
        TextSize::SMALL,
        disabled);

    // --------------------------------------------------------
    // UPDATE DISPLAY
    // --------------------------------------------------------

    DisplayManager::update();
}