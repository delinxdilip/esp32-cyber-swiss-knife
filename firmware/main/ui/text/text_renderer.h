#pragma once

#include <cstdint>

// ------------------------------------------------------------
// TEXT SIZE
// ------------------------------------------------------------

enum class TextSize : uint8_t
{
    SMALL = 1,
    MEDIUM = 2,
    LARGE = 3
};

// ------------------------------------------------------------
// TEXT RENDERER
// ------------------------------------------------------------

class TextRenderer
{
public:
    /**
     * Draw text using the selected bitmap font size.
     *
     * Text is rendered using the display manager and is
     * written into the display framebuffer.
     *
     * @param text Null-terminated text string.
     * @param x Starting X coordinate.
     * @param y Starting Y coordinate.
     * @param size Bitmap scale.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    static void draw(
        const char *text,
        int16_t x,
        int16_t y,
        TextSize size,
        uint32_t color);

    /**
     * Calculate the width of a text string.
     *
     * @param text Null-terminated text string.
     * @param size Bitmap scale.
     * @return Width in pixels.
     */
    static uint16_t get_width(
        const char *text,
        TextSize size);

    /**
     * Get the height of a text string.
     *
     * @param size Bitmap scale.
     * @return Height in pixels.
     */
    static uint16_t get_height(
        TextSize size);
};