#pragma once

#include <cstdint>

// ------------------------------------------------------------
// UI LAYOUT
// ------------------------------------------------------------
//
// Provides display-independent geometry for the UI.
//
// UILayout reads the configured TFT dimensions and derives
// common geometry such as screen center and safe drawing area.
//
// Screen-specific layout decisions remain inside each screen.
//
// ------------------------------------------------------------

class UILayout
{
public:
    static void init();

    // --------------------------------------------------------
    // DISPLAY DIMENSIONS
    // --------------------------------------------------------

    static uint16_t width();
    static uint16_t height();

    // --------------------------------------------------------
    // SCREEN CENTER
    // --------------------------------------------------------

    static int16_t center_x();
    static int16_t center_y();

    // --------------------------------------------------------
    // SAFE AREA
    // --------------------------------------------------------

    static int16_t safe_left();
    static int16_t safe_right();
    static int16_t safe_top();
    static int16_t safe_bottom();

    static uint16_t safe_width();
    static uint16_t safe_height();

private:
    static uint16_t screen_width;
    static uint16_t screen_height;

    static int16_t safe_area_left;
    static int16_t safe_area_right;
    static int16_t safe_area_top;
    static int16_t safe_area_bottom;

    static void calculate_safe_area();
};