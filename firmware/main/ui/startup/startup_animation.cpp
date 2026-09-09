#include "startup_animation.h"

#include <cmath>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ui/display/display_manager.h"

// ------------------------------------------------------------
// CYBERBOOT CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr uint16_t FRAME_WIDTH =
        240;

    constexpr uint16_t FRAME_HEIGHT =
        240;

    constexpr uint16_t FRAME_COUNT =
        60;

    constexpr uint32_t FRAME_DELAY_MS =
        33;

    constexpr uint32_t COLOR_BLACK =
        0x000000;

    constexpr uint32_t COLOR_WHITE =
        0xFFFFFF;

    constexpr float PI =
        3.14159265358979323846f;
}

// ------------------------------------------------------------
// DRAW RADIAL GLYPH
// ------------------------------------------------------------

void draw_radial_glyph(
    float progress)
{
    const int16_t center_x =
        FRAME_WIDTH / 2;

    const int16_t center_y =
        FRAME_HEIGHT / 2;

    const float radius =
        8.0f +
        (progress * 42.0f);

    constexpr uint16_t SPOKE_COUNT =
        8;

    for (uint16_t index = 0;
         index < SPOKE_COUNT;
         index++)
    {
        float angle =
            (2.0f * PI * index / SPOKE_COUNT) -
            (PI / 2.0f);

        float inner_radius =
            radius * 0.45f;

        float outer_radius =
            radius;

        int16_t x0 =
            center_x +
            static_cast<int16_t>(
                std::cos(angle) * inner_radius);

        int16_t y0 =
            center_y +
            static_cast<int16_t>(
                std::sin(angle) * inner_radius);

        int16_t x1 =
            center_x +
            static_cast<int16_t>(
                std::cos(angle) * outer_radius);

        int16_t y1 =
            center_y +
            static_cast<int16_t>(
                std::sin(angle) * outer_radius);

        DisplayManager::draw_line(
            x0,
            y0,
            x1,
            y1,
            COLOR_WHITE);
    }

    DisplayManager::draw_circle(
        center_x,
        center_y,
        static_cast<uint16_t>(radius),
        COLOR_WHITE);

    DisplayManager::fill_circle(
        center_x,
        center_y,
        4,
        COLOR_WHITE);
}

// ------------------------------------------------------------
// DRAW TOOL MARKERS
// ------------------------------------------------------------

void draw_tool_markers(
    float progress)
{
    const int16_t center_x =
        FRAME_WIDTH / 2;

    const int16_t center_y =
        FRAME_HEIGHT / 2;

    constexpr uint16_t MARKER_COUNT =
        4;

    constexpr uint16_t MARKER_RADIUS =
        67;

    for (uint16_t index = 0;
         index < MARKER_COUNT;
         index++)
    {
        float marker_progress =
            progress -
            (static_cast<float>(index) * 0.12f);

        if (marker_progress <= 0.0f)
        {
            continue;
        }

        if (marker_progress > 1.0f)
        {
            marker_progress = 1.0f;
        }

        float angle =
            (2.0f * PI * index / MARKER_COUNT) -
            (PI / 2.0f);

        int16_t x =
            center_x +
            static_cast<int16_t>(
                std::cos(angle) *
                MARKER_RADIUS *
                marker_progress);

        int16_t y =
            center_y +
            static_cast<int16_t>(
                std::sin(angle) *
                MARKER_RADIUS *
                marker_progress);

        DisplayManager::fill_circle(
            x,
            y,
            3,
            COLOR_WHITE);
    }
}

// ------------------------------------------------------------
// DRAW CYBER MARK
// ------------------------------------------------------------

void draw_cyber_mark(
    float progress)
{
    if (progress <= 0.0f)
    {
        return;
    }

    if (progress > 1.0f)
    {
        progress = 1.0f;
    }

    const int16_t center_x =
        FRAME_WIDTH / 2;

    const int16_t center_y =
        FRAME_HEIGHT / 2;

    const int16_t line_y =
        center_y + 63;

    const int16_t line_width =
        static_cast<int16_t>(
            24 +
            (progress * 56));

    DisplayManager::fill_rect(
        center_x - (line_width / 2),
        line_y,
        line_width,
        2,
        COLOR_WHITE);
}

// ------------------------------------------------------------
// DRAW FINAL FRAME
// ------------------------------------------------------------

void draw_final_mark()
{
    const int16_t center_x =
        FRAME_WIDTH / 2;

    const int16_t center_y =
        FRAME_HEIGHT / 2;

    DisplayManager::draw_circle(
        center_x,
        center_y,
        48,
        COLOR_WHITE);

    DisplayManager::fill_rect(
        center_x - 30,
        center_y - 1,
        60,
        2,
        COLOR_WHITE);

    DisplayManager::fill_rect(
        center_x - 1,
        center_y - 30,
        2,
        60,
        COLOR_WHITE);

    DisplayManager::fill_circle(
        center_x,
        center_y,
        5,
        COLOR_WHITE);
}

// ------------------------------------------------------------
// CYBERBOOT FRAME
// ------------------------------------------------------------

void draw_frame(
    uint16_t frame)
{
    float progress =
        static_cast<float>(frame) /
        static_cast<float>(FRAME_COUNT - 1);

    DisplayManager::clear(
        COLOR_BLACK);

    // --------------------------------------------------------
    // CORE GLYPH
    // --------------------------------------------------------

    float glyph_progress =
        progress < 0.60f
            ? progress / 0.60f
            : 1.0f;

    draw_radial_glyph(
        glyph_progress);

    // --------------------------------------------------------
    // TOOL MARKERS
    // --------------------------------------------------------

    float marker_progress =
        progress < 0.78f
            ? progress / 0.78f
            : 1.0f;

    draw_tool_markers(
        marker_progress);

    // --------------------------------------------------------
    // CYBER MARK
    // --------------------------------------------------------

    float mark_progress =
        progress < 0.72f
            ? 0.0f
            : (progress - 0.72f) / 0.28f;

    draw_cyber_mark(
        mark_progress);

    // --------------------------------------------------------
    // FINAL GLYPH
    // --------------------------------------------------------

    if (progress >= 0.90f)
    {
        DisplayManager::clear(
            COLOR_BLACK);

        draw_final_mark();

        DisplayManager::update();

        return;
    }

    DisplayManager::update();
}

// ------------------------------------------------------------
// PLAY
// ------------------------------------------------------------

void StartupAnimation::play()
{
    if (!DisplayManager::is_initialized() ||
        !DisplayManager::has_driver())
    {
        return;
    }

    for (uint16_t frame = 0;
         frame < FRAME_COUNT;
         frame++)
    {
        draw_frame(
            frame);

        vTaskDelay(
            pdMS_TO_TICKS(
                FRAME_DELAY_MS));
    }

    // --------------------------------------------------------
    // FINAL CLEAR
    // --------------------------------------------------------

    DisplayManager::clear(
        COLOR_BLACK);

    DisplayManager::update();
}