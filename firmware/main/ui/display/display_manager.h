#pragma once

#include <cstdint>

#include "display_driver.h"

/**
 * Display Manager
 *
 * Provides the single entry point for the rest of the firmware
 * to interact with the display subsystem.
 *
 * DisplayManager is responsible for:
 *
 * - Reading the configured TFT controller
 * - Selecting the appropriate display driver
 * - Initializing and deinitializing the driver
 * - Providing generic display operations
 *
 * Controller-specific implementation details must remain inside
 * the corresponding driver.
 *
 * Colors exposed by the display manager use 24-bit RGB values
 * in the format:
 *
 *     0xRRGGBB
 */
class DisplayManager
{
public:
    /**
     * Initialize the display subsystem using the configuration
     * stored in ConfigManager.
     *
     * If TFT is disabled in the configuration, initialization
     * succeeds without creating a display driver.
     *
     * @return true if initialization succeeds or TFT is disabled.
     */
    static bool initialize();

    /**
     * Deinitialize the active display driver and release
     * display resources.
     */
    static void deinitialize();

    /**
     * Clear the display using the specified RGB color.
     *
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    static void clear(uint32_t color);

    /**
     * Set the display rotation.
     *
     * @param rotation Desired display rotation.
     * @return true if the rotation was applied successfully.
     */
    static bool set_rotation(TFTRotation rotation);

    /**
     * Draw a single pixel.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     * @return true if the pixel was drawn successfully.
     */
    static bool draw_pixel(
        uint16_t x,
        uint16_t y,
        uint32_t color);

    /**
     * Draw a line between two points.
     *
     * @param x0 Starting X coordinate.
     * @param y0 Starting Y coordinate.
     * @param x1 Ending X coordinate.
     * @param y1 Ending Y coordinate.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    static void draw_line(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        uint32_t color);

    /**
     * Draw an unfilled rectangle.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param width Rectangle width.
     * @param height Rectangle height.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    static void draw_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint32_t color);

    /**
     * Draw a filled rectangle.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param width Rectangle width.
     * @param height Rectangle height.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    static void fill_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint32_t color);

    /**
     * Draw an unfilled circle.
     *
     * @param center_x Circle center X coordinate.
     * @param center_y Circle center Y coordinate.
     * @param radius Circle radius.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    static void draw_circle(
        int16_t center_x,
        int16_t center_y,
        uint16_t radius,
        uint32_t color);

    /**
     * Draw a filled circle.
     *
     * @param center_x Circle center X coordinate.
     * @param center_y Circle center Y coordinate.
     * @param radius Circle radius.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    static void fill_circle(
        int16_t center_x,
        int16_t center_y,
        uint16_t radius,
        uint32_t color);

    /**
     * Update the display.
     *
     * The active driver determines how display updates are
     * synchronized or flushed.
     */
    static void update();

    /**
     * Check whether the display subsystem is initialized.
     *
     * @return true when the display subsystem is ready.
     */
    static bool is_initialized();

    /**
     * Check whether a display driver is currently active.
     *
     * @return true if a driver has been selected.
     */
    static bool has_driver();

    /**
     * Get the active display driver.
     *
     * This should only be used by display-layer code that needs
     * access to functionality exposed by the generic driver.
     *
     * @return Pointer to the active driver, or nullptr.
     */
    static DisplayDriver *get_driver();

private:
    /**
     * Create the appropriate display driver based on the
     * configured TFT controller.
     *
     * @return true if a supported driver was created.
     */
    static bool create_driver();

    /**
     * Destroy the active display driver.
     */
    static void destroy_driver();

    static DisplayDriver *driver;
    static bool initialized;
};