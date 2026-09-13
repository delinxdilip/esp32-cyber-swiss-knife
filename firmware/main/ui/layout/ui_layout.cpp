#include "ui_layout.h"

#include "core/config/config_manager.h"

// ------------------------------------------------------------
// STATIC STATE
// ------------------------------------------------------------

uint16_t UILayout::screen_width = 0;
uint16_t UILayout::screen_height = 0;

int16_t UILayout::safe_area_left = 0;
int16_t UILayout::safe_area_right = 0;
int16_t UILayout::safe_area_top = 0;
int16_t UILayout::safe_area_bottom = 0;

// ------------------------------------------------------------
// INITIALIZE
// ------------------------------------------------------------

void UILayout::init()
{
    const TFTConfig &config =
        ConfigManager::get_tft_config();

    screen_width =
        config.width;

    screen_height =
        config.height;

    calculate_safe_area();
}

// ------------------------------------------------------------
// WIDTH
// ------------------------------------------------------------

uint16_t UILayout::width()
{
    return screen_width;
}

// ------------------------------------------------------------
// HEIGHT
// ------------------------------------------------------------

uint16_t UILayout::height()
{
    return screen_height;
}

// ------------------------------------------------------------
// CENTER X
// ------------------------------------------------------------

int16_t UILayout::center_x()
{
    return static_cast<int16_t>(
        screen_width / 2);
}

// ------------------------------------------------------------
// CENTER Y
// ------------------------------------------------------------

int16_t UILayout::center_y()
{
    return static_cast<int16_t>(
        screen_height / 2);
}

// ------------------------------------------------------------
// SAFE LEFT
// ------------------------------------------------------------

int16_t UILayout::safe_left()
{
    return safe_area_left;
}

// ------------------------------------------------------------
// SAFE RIGHT
// ------------------------------------------------------------

int16_t UILayout::safe_right()
{
    return safe_area_right;
}

// ------------------------------------------------------------
// SAFE TOP
// ------------------------------------------------------------

int16_t UILayout::safe_top()
{
    return safe_area_top;
}

// ------------------------------------------------------------
// SAFE BOTTOM
// ------------------------------------------------------------

int16_t UILayout::safe_bottom()
{
    return safe_area_bottom;
}

// ------------------------------------------------------------
// SAFE WIDTH
// ------------------------------------------------------------

uint16_t UILayout::safe_width()
{
    if (safe_area_right < safe_area_left)
    {
        return 0;
    }

    return static_cast<uint16_t>(
        safe_area_right -
        safe_area_left +
        1);
}

// ------------------------------------------------------------
// SAFE HEIGHT
// ------------------------------------------------------------

uint16_t UILayout::safe_height()
{
    if (safe_area_bottom < safe_area_top)
    {
        return 0;
    }

    return static_cast<uint16_t>(
        safe_area_bottom -
        safe_area_top +
        1);
}

// ------------------------------------------------------------
// CALCULATE SAFE AREA
// ------------------------------------------------------------

void UILayout::calculate_safe_area()
{
    if (screen_width == 0 ||
        screen_height == 0)
    {
        safe_area_left = 0;
        safe_area_right = 0;
        safe_area_top = 0;
        safe_area_bottom = 0;

        return;
    }

    // --------------------------------------------------------
    // RECTANGULAR DISPLAY
    // --------------------------------------------------------

    if (ConfigManager::get_tft_config().shape ==
        TFTShape::RECTANGLE)
    {
        safe_area_left = 0;
        safe_area_right =
            static_cast<int16_t>(
                screen_width - 1);

        safe_area_top = 0;
        safe_area_bottom =
            static_cast<int16_t>(
                screen_height - 1);

        return;
    }

    // --------------------------------------------------------
    // ROUND DISPLAY
    // --------------------------------------------------------
    //
    // The safe area is the largest centered rectangle that
    // fits completely inside the circular display.
    //
    // For a square round display this becomes the inscribed
    // square.
    //
    // --------------------------------------------------------

    const uint16_t diameter =
        screen_width < screen_height
            ? screen_width
            : screen_height;

    const int16_t safe_size =
        static_cast<int16_t>(
            static_cast<float>(diameter) *
            0.70710678f);

    const int16_t horizontal_offset =
        static_cast<int16_t>(
            (screen_width - safe_size) / 2);

    const int16_t vertical_offset =
        static_cast<int16_t>(
            (screen_height - safe_size) / 2);

    safe_area_left =
        horizontal_offset;

    safe_area_right =
        horizontal_offset +
        safe_size -
        1;

    safe_area_top =
        vertical_offset;

    safe_area_bottom =
        vertical_offset +
        safe_size -
        1;
}