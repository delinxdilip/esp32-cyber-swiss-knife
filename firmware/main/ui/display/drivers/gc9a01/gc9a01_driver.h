#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#include "ui/display/display_driver.h"

// ------------------------------------------------------------
// GC9A01 DISPLAY DRIVER
// ------------------------------------------------------------

class GC9A01Driver : public DisplayDriver
{
public:
    GC9A01Driver();
    ~GC9A01Driver() override;

    bool initialize(
        const TFTConfig &config) override;

    void deinitialize() override;

    void clear(
        uint32_t color) override;

    bool set_rotation(
        TFTRotation rotation) override;

    bool draw_pixel(
        uint16_t x,
        uint16_t y,
        uint32_t color) override;

    void draw_line(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        uint32_t color) override;

    void draw_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint32_t color) override;

    void fill_rect(
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint32_t color) override;

    void draw_circle(
        int16_t center_x,
        int16_t center_y,
        uint16_t radius,
        uint32_t color) override;

    void fill_circle(
        int16_t center_x,
        int16_t center_y,
        uint16_t radius,
        uint32_t color) override;

    void update() override;

    bool is_initialized() const override;

    uint16_t get_width() const override;

    uint16_t get_height() const override;

private:
    uint16_t rgb888_to_rgb565(
        uint32_t color);

    void put_pixel(
        int16_t x,
        int16_t y,
        uint16_t color);

    void draw_horizontal_span(
        int16_t x0,
        int16_t x1,
        int16_t y,
        uint16_t color);

private:
    bool initialized;

    uint16_t width;
    uint16_t height;

    TFTRotation rotation;

    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;

    uint8_t *framebuffer;
};