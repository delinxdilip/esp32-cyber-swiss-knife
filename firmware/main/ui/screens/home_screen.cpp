#include "home_screen.h"

#include <cstdio>

#include "ui/display/display_manager.h"
#include "ui/layout/ui_layout.h"
#include "ui/text/text_renderer.h"
#include "ui/theme/ui_theme.h"

// ------------------------------------------------------------
// HOME SCREEN CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr int16_t EDGE_PADDING = 2;

    constexpr int16_t HEADER_GAP = 6;
    constexpr int16_t TIME_GAP = 10;
    constexpr int16_t FOOTER_GAP = 6;

    constexpr uint8_t ROW_COUNT = 5;
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
            UILayout::center_x() -
            static_cast<int16_t>(
                width / 2);

        TextRenderer::draw(
            text,
            x,
            y,
            size,
            color);
    }

    void draw_left(
        const char *text,
        int16_t y,
        TextSize size,
        uint32_t color)
    {
        TextRenderer::draw(
            text,
            UILayout::safe_left() +
                EDGE_PADDING,
            y,
            size,
            color);
    }

    void draw_right(
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
            UILayout::safe_right() -
            EDGE_PADDING -
            static_cast<int16_t>(
                width) +
            1;

        TextRenderer::draw(
            text,
            x,
            y,
            size,
            color);
    }
}

// ------------------------------------------------------------
// WIFI STATE
// ------------------------------------------------------------

namespace
{
    const char *get_wifi_state(
        const UIModel &model)
    {
        if (!model.wifi_enabled)
        {
            return "OFF";
        }

        if (model.wifi_connected)
        {
            return "CON";
        }

        return "ON";
    }
}

// ------------------------------------------------------------
// BLUETOOTH STATE
// ------------------------------------------------------------

namespace
{
    const char *get_bluetooth_state(
        const UIModel &model)
    {
        if (!model.bluetooth_enabled)
        {
            return "OFF";
        }

        if (model.bluetooth_connected)
        {
            return "CON";
        }

        return "ON";
    }
}

// ------------------------------------------------------------
// ACCESS POINT STATE
// ------------------------------------------------------------

namespace
{
    void draw_ap_state(
        const UIModel &model,
        int16_t y,
        TextSize size,
        uint32_t primary,
        uint32_t disabled)
    {
        // ----------------------------------------------------
        // AP OFF
        // ----------------------------------------------------

        if (!model.ap_running)
        {
            draw_right(
                "OFF",
                y,
                size,
                disabled);

            return;
        }

        // ----------------------------------------------------
        // AP ON WITH NO CLIENTS
        // ----------------------------------------------------

        if (model.ap_clients == 0)
        {
            draw_right(
                "ON",
                y,
                size,
                primary);

            return;
        }

        // ----------------------------------------------------
        // AP WITH CONNECTED CLIENTS
        // ----------------------------------------------------

        char clients[8];

        std::snprintf(
            clients,
            sizeof(clients),
            "%02u/%02u",
            static_cast<unsigned int>(
                model.ap_clients),
            static_cast<unsigned int>(
                model.ap_max_connections));

        draw_right(
            clients,
            y,
            size,
            primary);
    }
}

// ------------------------------------------------------------
// HOME LAYOUT
// ------------------------------------------------------------

namespace
{
    struct HomeLayout
    {
        int16_t header_y;

        int16_t time_y;

        int16_t temperature_y;
        int16_t wifi_y;
        int16_t ble_y;
        int16_t ap_y;
        int16_t uptime_y;

        int16_t footer_y;
    };

    HomeLayout calculate_layout()
    {
        HomeLayout layout{};

        const int16_t safe_top =
            UILayout::safe_top();

        const int16_t safe_bottom =
            UILayout::safe_bottom();

        // ----------------------------------------------------
        // TEXT HEIGHTS
        // ----------------------------------------------------

        const int16_t small_height =
            static_cast<int16_t>(
                TextRenderer::get_height(
                    TextSize::SMALL));

        const int16_t large_height =
            static_cast<int16_t>(
                TextRenderer::get_height(
                    TextSize::LARGE));

        // ----------------------------------------------------
        // HEADER
        // ----------------------------------------------------
        //
        // CSK is physically anchored to the top of the
        // configured safe area.
        //
        // ----------------------------------------------------

        layout.header_y =
            safe_top;

        // ----------------------------------------------------
        // FOOTER
        // ----------------------------------------------------
        //
        // Firmware version is physically anchored to the
        // bottom of the configured safe area.
        //
        // ----------------------------------------------------

        layout.footer_y =
            safe_bottom -
            small_height +
            1;

        // ----------------------------------------------------
        // TIME
        // ----------------------------------------------------

        layout.time_y =
            layout.header_y +
            small_height +
            HEADER_GAP;

        // ----------------------------------------------------
        // INFORMATION AREA
        // ----------------------------------------------------
        //
        // The five information rows are distributed evenly
        // between the end of the time element and the start
        // of the footer.
        //
        // ----------------------------------------------------

        const int16_t rows_top =
            layout.time_y +
            large_height +
            TIME_GAP;

        const int16_t rows_bottom =
            layout.footer_y -
            FOOTER_GAP -
            small_height +
            1;

        const int16_t rows_height =
            rows_bottom -
            rows_top;

        const int16_t row_spacing =
            ROW_COUNT > 1
                ? rows_height /
                    static_cast<int16_t>(
                        ROW_COUNT - 1)
                : 0;

        layout.temperature_y =
            rows_top;

        layout.wifi_y =
            rows_top +
            row_spacing;

        layout.ble_y =
            rows_top +
            (row_spacing * 2);

        layout.ap_y =
            rows_top +
            (row_spacing * 3);

        layout.uptime_y =
            rows_top +
            (row_spacing * 4);

        return layout;
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
    // LAYOUT
    // --------------------------------------------------------

    const HomeLayout layout =
        calculate_layout();

    // --------------------------------------------------------
    // BACKGROUND
    // --------------------------------------------------------

    DisplayManager::clear(
        background);

    // --------------------------------------------------------
    // HEADER
    // --------------------------------------------------------

    draw_centered(
        "CSK",
        layout.header_y,
        TextSize::SMALL,
        primary);

    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    draw_centered(
        model.time,
        layout.time_y,
        TextSize::LARGE,
        primary);

    // --------------------------------------------------------
    // TEMPERATURE
    // --------------------------------------------------------

    char temperature[8];

    std::snprintf(
        temperature,
        sizeof(temperature),
        "%uC",
        static_cast<unsigned int>(
            model.temperature_celsius));

    draw_left(
        "TEMP",
        layout.temperature_y,
        TextSize::SMALL,
        secondary);

    draw_right(
        temperature,
        layout.temperature_y,
        TextSize::SMALL,
        primary);

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    draw_left(
        "WIFI",
        layout.wifi_y,
        TextSize::SMALL,
        secondary);

    const char *wifi_state =
        get_wifi_state(model);

    draw_right(
        wifi_state,
        layout.wifi_y,
        TextSize::SMALL,
        model.wifi_enabled
            ? primary
            : disabled);

    // --------------------------------------------------------
    // BLUETOOTH
    // --------------------------------------------------------

    draw_left(
        "BLE",
        layout.ble_y,
        TextSize::SMALL,
        secondary);

    const char *bluetooth_state =
        get_bluetooth_state(model);

    draw_right(
        bluetooth_state,
        layout.ble_y,
        TextSize::SMALL,
        model.bluetooth_enabled
            ? primary
            : disabled);

    // --------------------------------------------------------
    // ACCESS POINT
    // --------------------------------------------------------

    draw_left(
        "AP",
        layout.ap_y,
        TextSize::SMALL,
        secondary);

    draw_ap_state(
        model,
        layout.ap_y,
        TextSize::SMALL,
        primary,
        disabled);

    // --------------------------------------------------------
    // UPTIME
    // --------------------------------------------------------

    char uptime[16];

    const uint32_t days =
        model.uptime_seconds / 86400;

    const uint32_t hours =
        (model.uptime_seconds % 86400) / 3600;

    const uint32_t minutes =
        (model.uptime_seconds % 3600) / 60;

    std::snprintf(
        uptime,
        sizeof(uptime),
        "%lud:%luh:%lum",
        static_cast<unsigned long>(
            days),
        static_cast<unsigned long>(
            hours),
        static_cast<unsigned long>(
            minutes));

    draw_left(
        "UPTIME",
        layout.uptime_y,
        TextSize::SMALL,
        secondary);

    draw_right(
        uptime,
        layout.uptime_y,
        TextSize::SMALL,
        primary);

    // --------------------------------------------------------
    // FOOTER
    // --------------------------------------------------------

    draw_centered(
        model.firmware_version,
        layout.footer_y,
        TextSize::SMALL,
        disabled);

    // --------------------------------------------------------
    // UPDATE DISPLAY
    // --------------------------------------------------------

    DisplayManager::update();
}