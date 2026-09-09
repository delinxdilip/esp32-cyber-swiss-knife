#include "display_test.h"

#include "ui/display/display_manager.h"

// ------------------------------------------------------------
// DISPLAY TEST
// ------------------------------------------------------------

void DisplayTest::render()
{
    if (!DisplayManager::is_initialized() ||
        !DisplayManager::has_driver())
    {
        return;
    }

    const uint16_t width =
        DisplayManager::get_driver()->get_width();

    const uint16_t height =
        DisplayManager::get_driver()->get_height();

    const int16_t center_x =
        width / 2;

    const int16_t center_y =
        height / 2;

    // --------------------------------------------------------
    // CLEAR DISPLAY
    // --------------------------------------------------------

    // RGB888:
    // 0x000000 = BLACK
    DisplayManager::clear(
        0xFF0000);

    // --------------------------------------------------------
    // CENTER WHITE SQUARE
    // --------------------------------------------------------

    // RGB888:
    // 0xFFFFFF = WHITE
    const uint16_t square_size =
        60;

    const int16_t square_x =
        center_x -
        (square_size / 2);

    const int16_t square_y =
        center_y -
        (square_size / 2);

    DisplayManager::fill_rect(
        square_x,
        square_y,
        square_size,
        square_size,
        0x00FF00);

    // --------------------------------------------------------
    // UPDATE DISPLAY
    // --------------------------------------------------------

    DisplayManager::update();
}