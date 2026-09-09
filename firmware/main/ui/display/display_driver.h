#pragma once

#include <cstdint>

#include "display_types.h"

/**
 * Generic display driver interface.
 *
 * Every supported display controller must implement
 * this interface.
 *
 * The rest of the firmware should communicate with
 * displays through this interface rather than directly
 * accessing a controller-specific implementation.
 *
 * Colors exposed through this interface use 24-bit RGB
 * values in the format:
 *
 *     0xRRGGBB
 *
 * Controller-specific color conversion must remain inside
 * the individual display driver implementation.
 */
class DisplayDriver
{
public:
    virtual ~DisplayDriver() = default;

    /**
     * Initialize the physical display.
     *
     * @param config Generic TFT configuration.
     * @return true if initialization succeeds.
     */
    virtual bool initialize(const TFTConfig &config) = 0;

    /**
     * Deinitialize the display and release resources.
     */
    virtual void deinitialize() = 0;

    /**
     * Clear the complete display using the specified RGB color.
     *
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    virtual void clear(uint32_t color) = 0;

    /**
     * Set the display rotation.
     *
     * @param rotation Desired display rotation.
     * @return true if the rotation is supported and applied.
     */
    virtual bool set_rotation(TFTRotation rotation) = 0;

    /**
     * Draw a single pixel.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     * @return true if the pixel was drawn successfully.
     */
    virtual bool draw_pixel(
        uint16_t x,
        uint16_t y,
        uint32_t color) = 0;

    /**
     * Draw a line between two points.
     *
     * @param x0 Starting X coordinate.
     * @param y0 Starting Y coordinate.
     * @param x1 Ending X coordinate.
     * @param y1 Ending Y coordinate.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    virtual void draw_line(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        uint32_t color) = 0;

    /**
     * Draw an unfilled rectangle.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param width Rectangle width.
     * @param height Rectangle height.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    virtual void draw_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint32_t color) = 0;

    /**
     * Draw a filled rectangle.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param width Rectangle width.
     * @param height Rectangle height.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    virtual void fill_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint32_t color) = 0;

    /**
     * Draw an unfilled circle.
     *
     * @param center_x Circle center X coordinate.
     * @param center_y Circle center Y coordinate.
     * @param radius Circle radius.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    virtual void draw_circle(
        int16_t center_x,
        int16_t center_y,
        uint16_t radius,
        uint32_t color) = 0;

    /**
     * Draw a filled circle.
     *
     * @param center_x Circle center X coordinate.
     * @param center_y Circle center Y coordinate.
     * @param radius Circle radius.
     * @param color 24-bit RGB color in 0xRRGGBB format.
     */
    virtual void fill_circle(
        int16_t center_x,
        int16_t center_y,
        uint16_t radius,
        uint32_t color) = 0;

    /**
     * Push pending display data to the physical display.
     *
     * The implementation decides how buffering and
     * synchronization are handled.
     */
    virtual void update() = 0;

    /**
     * Check whether the driver has been initialized.
     *
     * @return true when the display is ready.
     */
    virtual bool is_initialized() const = 0;

    /**
     * Get the configured display width.
     *
     * @return Display width in pixels.
     */
    virtual uint16_t get_width() const = 0;

    /**
     * Get the configured display height.
     *
     * @return Display height in pixels.
     */
    virtual uint16_t get_height() const = 0;
};