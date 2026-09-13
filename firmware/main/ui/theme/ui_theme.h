#pragma once

#include <cstdint>

// ------------------------------------------------------------
// UI THEME
// ------------------------------------------------------------
//
// The UI theme is derived from the configured TFT theme color.
//
// Background remains permanently black.
// Primary UI elements use the configured theme color.
// Secondary and disabled colors are derived automatically.
//
// ------------------------------------------------------------

class UITheme
{
public:
    static void init();

    // --------------------------------------------------------
    // COLORS
    // --------------------------------------------------------

    static uint32_t background();

    static uint32_t primary();

    static uint32_t secondary();

    static uint32_t disabled();

    static uint32_t selection();

    static uint32_t border();

private:
    static uint32_t theme_color;

    static uint32_t scale_color(
        uint32_t color,
        uint8_t percentage);
};