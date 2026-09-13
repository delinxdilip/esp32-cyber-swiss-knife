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

    constexpr int16_t FOOTER_Y = 218;
}

// ------------------------------------------------------------
// DRAW CENTERED TEXT
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
// RENDER WIFI LANDING SCREEN
// ------------------------------------------------------------

void WiFiScreen::render_main()
{
    DisplayManager::clear(
        UITheme::background());

    draw_centered(
        "<",
        TITLE_Y,
        TextSize::MEDIUM,
        UITheme::secondary());

    draw_centered(
        "WI-FI",
        TITLE_Y,
        TextSize::MEDIUM,
        UITheme::primary());

    draw_centered(
        ">",
        TITLE_Y,
        TextSize::MEDIUM,
        UITheme::secondary());

    draw_centered(
        "PRESS TO OPEN",
        198,
        TextSize::SMALL,
        UITheme::secondary());

    DisplayManager::update();
}

// ------------------------------------------------------------
// RENDER WIFI SUBMENU
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