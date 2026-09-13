#include "text_renderer.h"

#include <cctype>

#include "ui/display/display_manager.h"

#include "fonts/font_5x7.h"

// ------------------------------------------------------------
// TEXT RENDERER CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr uint16_t FONT_WIDTH =
        Font5x7::GLYPH_WIDTH;

    constexpr uint16_t FONT_HEIGHT =
        Font5x7::GLYPH_HEIGHT;

    constexpr uint16_t FONT_SPACING =
        Font5x7::GLYPH_SPACING;

    constexpr uint16_t get_scale(
        TextSize size)
    {
        return static_cast<uint16_t>(
            size);
    }
}

// ------------------------------------------------------------
// DRAW CHARACTER
// ------------------------------------------------------------

void draw_character(
    char character,
    int16_t x,
    int16_t y,
    uint16_t scale,
    uint32_t color)
{
    character =
        static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(
                    character)));

    const uint8_t *glyph =
        Font5x7::get(
            character);

    for (uint16_t column = 0;
         column < FONT_WIDTH;
         column++)
    {
        uint8_t column_data =
            glyph[column];

        for (uint16_t row = 0;
             row < FONT_HEIGHT;
             row++)
        {
            if ((column_data & (1 << row)) == 0)
            {
                continue;
            }

            DisplayManager::fill_rect(
                x +
                    static_cast<int16_t>(
                        column * scale),
                y +
                    static_cast<int16_t>(
                        row * scale),
                scale,
                scale,
                color);
        }
    }
}

// ------------------------------------------------------------
// DRAW TEXT
// ------------------------------------------------------------

void TextRenderer::draw(
    const char *text,
    int16_t x,
    int16_t y,
    TextSize size,
    uint32_t color)
{
    if (text == nullptr)
    {
        return;
    }

    if (!DisplayManager::is_initialized() ||
        !DisplayManager::has_driver())
    {
        return;
    }

    const uint16_t scale =
        get_scale(size);

    const uint16_t character_width =
        (FONT_WIDTH + FONT_SPACING) *
        scale;

    int16_t cursor_x =
        x;

    while (*text != '\0')
    {
        draw_character(
            *text,
            cursor_x,
            y,
            scale,
            color);

        cursor_x +=
            character_width;

        text++;
    }
}

// ------------------------------------------------------------
// GET WIDTH
// ------------------------------------------------------------

uint16_t TextRenderer::get_width(
    const char *text,
    TextSize size)
{
    if (text == nullptr)
    {
        return 0;
    }

    const uint16_t scale =
        get_scale(size);

    uint16_t character_count =
        0;

    while (*text != '\0')
    {
        character_count++;

        text++;
    }

    if (character_count == 0)
    {
        return 0;
    }

    return static_cast<uint16_t>(
        ((FONT_WIDTH + FONT_SPACING) *
         character_count -
         FONT_SPACING) *
        scale);
}

// ------------------------------------------------------------
// GET HEIGHT
// ------------------------------------------------------------

uint16_t TextRenderer::get_height(
    TextSize size)
{
    return static_cast<uint16_t>(
        FONT_HEIGHT *
        get_scale(size));
}