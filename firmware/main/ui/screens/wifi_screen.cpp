#include "wifi_screen.h"

#include "ui/display/display_manager.h"
#include "ui/text/text_renderer.h"
#include "ui/theme/ui_theme.h"

// ------------------------------------------------------------
// WIFI SCREEN CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr int16_t SCREEN_WIDTH = 240;

    constexpr int16_t TITLE_Y = 20;

    constexpr int16_t MENU_START_Y = 70;
    constexpr int16_t MENU_SPACING = 32;

    constexpr int16_t DETAIL_LABEL_Y = 72;
    constexpr int16_t DETAIL_VALUE_Y = 104;

    constexpr int16_t FOOTER_Y = 218;
}

// ------------------------------------------------------------
// DRAW CENTERED
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
// DRAW TITLE
// ------------------------------------------------------------

namespace
{
    void draw_title(
        const char *title)
    {
        draw_centered(
            title,
            TITLE_Y,
            TextSize::MEDIUM,
            UITheme::primary());
    }
}

// ------------------------------------------------------------
// DRAW MENU ITEM
// ------------------------------------------------------------

namespace
{
    void draw_menu_item(
        const char *text,
        uint8_t index,
        uint8_t selected_item)
    {
        const int16_t y =
            MENU_START_Y +
            (static_cast<int16_t>(index) *
             MENU_SPACING);

        const bool selected =
            index == selected_item;

        const uint32_t color =
            selected
                ? UITheme::primary()
                : UITheme::secondary();

        if (selected)
        {
            DisplayManager::fill_rect(
                28,
                y - 4,
                184,
                24,
                UITheme::selection());
        }

        TextRenderer::draw(
            text,
            40,
            y,
            TextSize::SMALL,
            color);
    }
}

// ------------------------------------------------------------
// RENDER MAIN
// ------------------------------------------------------------

void WiFiScreen::render_main()
{
    DisplayManager::clear(
        UITheme::background());

    draw_title(
        "WI-FI");

    draw_centered(
        "PRESS TO OPEN",
        106,
        TextSize::SMALL,
        UITheme::secondary());

    draw_centered(
        "LEFT TO BACK",
        142,
        TextSize::SMALL,
        UITheme::disabled());

    DisplayManager::update();
}

// ------------------------------------------------------------
// RENDER MENU
// ------------------------------------------------------------

void WiFiScreen::render_menu(
    uint8_t selected_item)
{
    DisplayManager::clear(
        UITheme::background());

    draw_title(
        "WI-FI");

    draw_menu_item(
        "STATUS",
        0,
        selected_item);

    draw_menu_item(
        "SCAN",
        1,
        selected_item);

    draw_menu_item(
        "NETWORKS",
        2,
        selected_item);

    draw_menu_item(
        "AP",
        3,
        selected_item);

    draw_centered(
        "UP/DOWN  SELECT",
        FOOTER_Y,
        TextSize::SMALL,
        UITheme::disabled());

    DisplayManager::update();
}

// ------------------------------------------------------------
// RENDER STATUS
// ------------------------------------------------------------

void WiFiScreen::render_status(
    const UIModel &model)
{
    DisplayManager::clear(
        UITheme::background());

    draw_title(
        "STATUS");

    TextRenderer::draw(
        "STATE",
        36,
        DETAIL_LABEL_Y,
        TextSize::SMALL,
        UITheme::secondary());

    TextRenderer::draw(
        model.wifi_connected
            ? "CONNECTED"
            : "OFFLINE",
        36,
        DETAIL_VALUE_Y,
        TextSize::SMALL,
        model.wifi_connected
            ? UITheme::primary()
            : UITheme::disabled());

    TextRenderer::draw(
        "SIGNAL",
        36,
        142,
        TextSize::SMALL,
        UITheme::secondary());

    TextRenderer::draw(
        "100%",
        150,
        142,
        TextSize::SMALL,
        UITheme::primary());

    draw_centered(
        "LEFT TO BACK",
        FOOTER_Y,
        TextSize::SMALL,
        UITheme::disabled());

    DisplayManager::update();
}

// ------------------------------------------------------------
// RENDER SCAN
// ------------------------------------------------------------

void WiFiScreen::render_scan()
{
    DisplayManager::clear(
        UITheme::background());

    draw_title(
        "SCAN");

    draw_centered(
        "READY",
        92,
        TextSize::MEDIUM,
        UITheme::primary());

    draw_centered(
        "PRESS TO START",
        132,
        TextSize::SMALL,
        UITheme::secondary());

    draw_centered(
        "LEFT TO BACK",
        FOOTER_Y,
        TextSize::SMALL,
        UITheme::disabled());

    DisplayManager::update();
}

// ------------------------------------------------------------
// RENDER NETWORKS
// ------------------------------------------------------------

void WiFiScreen::render_networks()
{
    DisplayManager::clear(
        UITheme::background());

    draw_title(
        "NETWORKS");

    TextRenderer::draw(
        "HOME_WIFI",
        32,
        76,
        TextSize::SMALL,
        UITheme::primary());

    TextRenderer::draw(
        "LAB_NETWORK",
        32,
        108,
        TextSize::SMALL,
        UITheme::secondary());

    TextRenderer::draw(
        "TEST_AP",
        32,
        140,
        TextSize::SMALL,
        UITheme::disabled());

    draw_centered(
        "3 NETWORKS",
        FOOTER_Y,
        TextSize::SMALL,
        UITheme::disabled());

    DisplayManager::update();
}

// ------------------------------------------------------------
// RENDER ACCESS POINT
// ------------------------------------------------------------

void WiFiScreen::render_ap(
    const UIModel &model)
{
    DisplayManager::clear(
        UITheme::background());

    draw_title(
        "AP");

    TextRenderer::draw(
        "STATE",
        36,
        DETAIL_LABEL_Y,
        TextSize::SMALL,
        UITheme::secondary());

    TextRenderer::draw(
        model.ap_running
            ? "ON"
            : "OFF",
        150,
        DETAIL_LABEL_Y,
        TextSize::SMALL,
        model.ap_running
            ? UITheme::primary()
            : UITheme::disabled());

    TextRenderer::draw(
        "CLIENTS",
        36,
        116,
        TextSize::SMALL,
        UITheme::secondary());

    TextRenderer::draw(
        "03",
        150,
        116,
        TextSize::SMALL,
        UITheme::primary());

    TextRenderer::draw(
        "CHANNEL",
        36,
        148,
        TextSize::SMALL,
        UITheme::secondary());

    TextRenderer::draw(
        "06",
        150,
        148,
        TextSize::SMALL,
        UITheme::primary());

    draw_centered(
        "LEFT TO BACK",
        FOOTER_Y,
        TextSize::SMALL,
        UITheme::disabled());

    DisplayManager::update();
}