#include "wifi_status_screen.h"

#include <cstdio>

#include "ui/display/display_manager.h"
#include "ui/layout/ui_layout.h"
#include "ui/theme/ui_theme.h"
#include "ui/text/text_renderer.h"

// ------------------------------------------------------------
// INTERNAL HELPERS
// ------------------------------------------------------------

namespace
{
    constexpr uint16_t SCREEN_WIDTH = 240;
    constexpr uint16_t ROW_HEIGHT = 23;
    constexpr uint16_t LABEL_WIDTH = 65;
    constexpr uint16_t HEADER_Y = 18;
    constexpr uint16_t CONTENT_START_Y = 50;
    constexpr uint16_t FOOTER_Y = 218;

    const char *get_wifi_state(const UIModel &model)
    {
        if (!model.wifi_enabled)
        {
            return "OFF";
        }

        if (model.wifi_connected)
        {
            return "CONNECTED";
        }

        return "DISCONNECTED";
    }

    const char *get_wifi_mode(const UIModel &model)
    {
        if (!model.wifi_enabled && !model.ap_running)
        {
            return "OFF";
        }

        if (model.wifi_enabled && model.ap_running)
        {
            return "STA+AP";
        }

        if (model.wifi_enabled)
        {
            return "STA";
        }

        if (model.ap_running)
        {
            return "AP";
        }

        return "OFF";
    }

    void draw_centered(
        const char *text,
        int16_t y,
        TextSize size,
        uint32_t color)
    {
        const uint16_t width =
            TextRenderer::get_width(text, size);

        const int16_t x =
            (SCREEN_WIDTH - width) / 2;

        TextRenderer::draw(
            text,
            x,
            y,
            size,
            color);
    }

    void draw_row(
        const char *label,
        const char *value,
        uint16_t y,
        bool disabled)
    {
        const uint32_t label_color =
            disabled
                ? UITheme::disabled()
                : UITheme::secondary();

        const uint32_t value_color =
            disabled
                ? UITheme::disabled()
                : UITheme::primary();

        TextRenderer::draw(
            label,
            UILayout::safe_left(),
            y,
            TextSize::SMALL,
            label_color);

        TextRenderer::draw(
            value,
            UILayout::safe_left() + LABEL_WIDTH,
            y,
            TextSize::SMALL,
            value_color);
    }

    void get_signal_text(
        const UIModel &model,
        char *buffer,
        size_t buffer_size)
    {
        if (!model.wifi_connected)
        {
            std::snprintf(buffer, buffer_size, "--");
            return;
        }

        std::snprintf(
            buffer,
            buffer_size,
            "%u%%",
            model.wifi_signal_percent);
    }

    void get_rssi_text(
        const UIModel &model,
        char *buffer,
        size_t buffer_size)
    {
        if (!model.wifi_connected)
        {
            std::snprintf(buffer, buffer_size, "--");
            return;
        }

        std::snprintf(
            buffer,
            buffer_size,
            "%d dBm",
            model.wifi_rssi);
    }

    void get_channel_text(
        const UIModel &model,
        char *buffer,
        size_t buffer_size)
    {
        if (!model.wifi_connected ||
            model.wifi_channel == 0)
        {
            std::snprintf(buffer, buffer_size, "--");
            return;
        }

        std::snprintf(
            buffer,
            buffer_size,
            "%u",
            model.wifi_channel);
    }
}

// ------------------------------------------------------------
// RENDER
// ------------------------------------------------------------

void WiFiStatusScreen::render(const UIModel &model)
{
    if (!DisplayManager::is_initialized() ||
        !DisplayManager::has_driver())
    {
        return;
    }

    DisplayManager::clear(
        UITheme::background());

    // --------------------------------------------------------
    // HEADER
    // --------------------------------------------------------

    draw_centered(
        "WIFI STATUS",
        HEADER_Y,
        TextSize::MEDIUM,
        UITheme::primary());

    // --------------------------------------------------------
    // VALUES
    // --------------------------------------------------------

    char signal_text[8];

    get_signal_text(
        model,
        signal_text,
        sizeof(signal_text));

    char rssi_text[12];

    get_rssi_text(
        model,
        rssi_text,
        sizeof(rssi_text));

    char channel_text[8];

    get_channel_text(
        model,
        channel_text,
        sizeof(channel_text));

    const bool disabled =
        !model.wifi_enabled &&
        !model.ap_running;

    const bool sta_connected =
        model.wifi_connected;

    // --------------------------------------------------------
    // MODE
    // --------------------------------------------------------

    draw_row(
        "MODE",
        get_wifi_mode(model),
        CONTENT_START_Y,
        disabled);

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    draw_row(
        "STATE",
        get_wifi_state(model),
        CONTENT_START_Y + ROW_HEIGHT,
        disabled);

    // --------------------------------------------------------
    // SSID
    // --------------------------------------------------------

    draw_row(
        "SSID",
        sta_connected
            ? model.wifi_ssid
            : "--",
        CONTENT_START_Y + ROW_HEIGHT * 2,
        disabled || !sta_connected);

    // --------------------------------------------------------
    // IP
    // --------------------------------------------------------

    draw_row(
        "IP",
        sta_connected
            ? model.wifi_ip_address
            : "--",
        CONTENT_START_Y + ROW_HEIGHT * 3,
        disabled || !sta_connected);

    // --------------------------------------------------------
    // RSSI
    // --------------------------------------------------------

    draw_row(
        "RSSI",
        rssi_text,
        CONTENT_START_Y + ROW_HEIGHT * 4,
        disabled || !sta_connected);

    // --------------------------------------------------------
    // SIGNAL
    // --------------------------------------------------------

    draw_row(
        "SIGNAL",
        signal_text,
        CONTENT_START_Y + ROW_HEIGHT * 5,
        disabled || !sta_connected);

    // --------------------------------------------------------
    // CHANNEL
    // --------------------------------------------------------

    draw_row(
        "CHANNEL",
        channel_text,
        CONTENT_START_Y + ROW_HEIGHT * 6,
        disabled || !sta_connected);

    // --------------------------------------------------------
    // SECURITY
    // --------------------------------------------------------

    draw_row(
        "SECURITY",
        sta_connected
            ? model.wifi_security
            : "--",
        CONTENT_START_Y + ROW_HEIGHT * 7,
        disabled || !sta_connected);

    // --------------------------------------------------------
    // FOOTER
    // --------------------------------------------------------

    draw_centered(
        "LEFT: BACK",
        FOOTER_Y,
        TextSize::SMALL,
        UITheme::secondary());

    DisplayManager::update();
}
