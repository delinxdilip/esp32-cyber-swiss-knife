#include "gc9a01_driver.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_lcd_gc9a01.h"
#include "esp_lcd_io_spi.h"

#include "core/logging/logger.h"

// ------------------------------------------------------------
// GC9A01 HARDWARE CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr spi_host_device_t LCD_HOST =
        SPI2_HOST;

    constexpr gpio_num_t LCD_SCLK_GPIO =
        GPIO_NUM_15;

    constexpr gpio_num_t LCD_MOSI_GPIO =
        GPIO_NUM_16;

    constexpr gpio_num_t LCD_DC_GPIO =
        GPIO_NUM_7;

    constexpr gpio_num_t LCD_CS_GPIO =
        GPIO_NUM_6;

    constexpr gpio_num_t LCD_RST_GPIO =
        GPIO_NUM_5;

    constexpr uint16_t LCD_WIDTH =
        240;

    constexpr uint16_t LCD_HEIGHT =
        240;

    constexpr uint16_t TEST_STRIP_HEIGHT =
        20;

    constexpr uint32_t SPI_CLOCK_HZ =
        20 * 1000 * 1000;

    constexpr size_t SPI_MAX_TRANSFER_SIZE =
        LCD_WIDTH *
        TEST_STRIP_HEIGHT *
        2;

    constexpr size_t FRAMEBUFFER_SIZE =
        LCD_WIDTH *
        LCD_HEIGHT *
        2;
}

// ------------------------------------------------------------
// CONSTRUCTOR / DESTRUCTOR
// ------------------------------------------------------------

GC9A01Driver::GC9A01Driver()
    : initialized(false),
      width(0),
      height(0),
      rotation(TFTRotation::ROTATION_0),
      io_handle(nullptr),
      panel_handle(nullptr),
      framebuffer(nullptr)
{
}

GC9A01Driver::~GC9A01Driver()
{
    deinitialize();
}

// ------------------------------------------------------------
// INITIALIZATION
// ------------------------------------------------------------

bool GC9A01Driver::initialize(
    const TFTConfig &config)
{
    if (initialized)
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "GC9A01 driver is already initialized");

        return true;
    }

    if (config.width != LCD_WIDTH ||
        config.height != LCD_HEIGHT)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Invalid GC9A01 resolution: %ux%u",
            config.width,
            config.height);

        return false;
    }

    width = LCD_WIDTH;
    height = LCD_HEIGHT;
    rotation = config.rotation;

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "Initializing GC9A01 display: %ux%u",
        width,
        height);

    // --------------------------------------------------------
    // SPI BUS
    // --------------------------------------------------------

    spi_bus_config_t bus_config = {};

    bus_config.sclk_io_num =
        LCD_SCLK_GPIO;

    bus_config.mosi_io_num =
        LCD_MOSI_GPIO;

    bus_config.miso_io_num =
        -1;

    bus_config.quadwp_io_num =
        -1;

    bus_config.quadhd_io_num =
        -1;

    bus_config.max_transfer_sz =
        SPI_MAX_TRANSFER_SIZE;

    esp_err_t error =
        spi_bus_initialize(
            LCD_HOST,
            &bus_config,
            SPI_DMA_CH_AUTO);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to initialize LCD SPI bus: %s",
            esp_err_to_name(error));

        return false;
    }

    // --------------------------------------------------------
    // PANEL IO
    // --------------------------------------------------------

    esp_lcd_panel_io_spi_config_t io_config = {};

    io_config.cs_gpio_num =
        LCD_CS_GPIO;

    io_config.dc_gpio_num =
        LCD_DC_GPIO;

    io_config.spi_mode =
        0;

    io_config.pclk_hz =
        SPI_CLOCK_HZ;

    io_config.trans_queue_depth =
        10;

    io_config.lcd_cmd_bits =
        8;

    io_config.lcd_param_bits =
        8;

    error =
        esp_lcd_new_panel_io_spi(
            static_cast<esp_lcd_spi_bus_handle_t>(
                LCD_HOST),
            &io_config,
            &io_handle);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to create GC9A01 panel IO: %s",
            esp_err_to_name(error));

        spi_bus_free(
            LCD_HOST);

        return false;
    }

    // --------------------------------------------------------
    // GC9A01 PANEL
    // --------------------------------------------------------

    esp_lcd_panel_dev_config_t panel_config = {};

    panel_config.reset_gpio_num =
        LCD_RST_GPIO;

    panel_config.rgb_ele_order =
        LCD_RGB_ELEMENT_ORDER_BGR;

    panel_config.bits_per_pixel =
        16;

    error =
        esp_lcd_new_panel_gc9a01(
            io_handle,
            &panel_config,
            &panel_handle);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to create GC9A01 panel: %s",
            esp_err_to_name(error));

        esp_lcd_panel_io_del(
            io_handle);

        io_handle = nullptr;

        spi_bus_free(
            LCD_HOST);

        return false;
    }

    // --------------------------------------------------------
    // PANEL RESET
    // --------------------------------------------------------

    error =
        esp_lcd_panel_reset(
            panel_handle);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to reset GC9A01 panel: %s",
            esp_err_to_name(error));

        deinitialize();

        return false;
    }

    // --------------------------------------------------------
    // PANEL INITIALIZATION
    // --------------------------------------------------------

    error =
        esp_lcd_panel_init(
            panel_handle);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to initialize GC9A01 panel: %s",
            esp_err_to_name(error));

        deinitialize();

        return false;
    }

    // --------------------------------------------------------
    // COLOR INVERSION
    // --------------------------------------------------------

    error =
        esp_lcd_panel_invert_color(
            panel_handle,
            true);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to disable GC9A01 color inversion: %s",
            esp_err_to_name(error));

        deinitialize();

        return false;
    }

    // --------------------------------------------------------
    // FRAMEBUFFER
    // --------------------------------------------------------

    framebuffer =
        static_cast<uint8_t *>(
            heap_caps_malloc(
                FRAMEBUFFER_SIZE,
                MALLOC_CAP_DMA |
                MALLOC_CAP_INTERNAL));

    if (framebuffer == nullptr)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to allocate GC9A01 framebuffer: %u bytes",
            static_cast<unsigned>(
                FRAMEBUFFER_SIZE));

        deinitialize();

        return false;
    }

    for (size_t index = 0;
         index < FRAMEBUFFER_SIZE;
         index++)
    {
        framebuffer[index] =
            0;
    }

    // --------------------------------------------------------
    // DISPLAY ROTATION
    // --------------------------------------------------------

    if (!set_rotation(rotation))
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to configure GC9A01 rotation");

        deinitialize();

        return false;
    }

    // --------------------------------------------------------
    // DISPLAY ON
    // --------------------------------------------------------

    error =
        esp_lcd_panel_disp_on_off(
            panel_handle,
            true);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to turn GC9A01 display on: %s",
            esp_err_to_name(error));

        deinitialize();

        return false;
    }

    initialized = true;

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "GC9A01 display initialized successfully");

    return true;
}

// ------------------------------------------------------------
// DEINITIALIZATION
// ------------------------------------------------------------

void GC9A01Driver::deinitialize()
{
    initialized = false;

    if (panel_handle != nullptr)
    {
        esp_lcd_panel_disp_on_off(
            panel_handle,
            false);

        esp_lcd_panel_del(
            panel_handle);

        panel_handle = nullptr;
    }

    if (io_handle != nullptr)
    {
        esp_lcd_panel_io_del(
            io_handle);

        io_handle = nullptr;
    }

    spi_bus_free(
        LCD_HOST);

    if (framebuffer != nullptr)
    {
        heap_caps_free(
            framebuffer);

        framebuffer = nullptr;
    }

    width = 0;
    height = 0;
    rotation = TFTRotation::ROTATION_0;
}

// ------------------------------------------------------------
// COLOR CONVERSION
// ------------------------------------------------------------

uint16_t GC9A01Driver::rgb888_to_rgb565(
    uint32_t color)
{
    uint8_t red =
        static_cast<uint8_t>(
            (color >> 16) & 0xFF);

    uint8_t green =
        static_cast<uint8_t>(
            (color >> 8) & 0xFF);

    uint8_t blue =
        static_cast<uint8_t>(
            color & 0xFF);

    return static_cast<uint16_t>(
        ((red & 0xF8) << 8) |
        ((green & 0xFC) << 3) |
        (blue >> 3));
}

// ------------------------------------------------------------
// CLEAR DISPLAY
// ------------------------------------------------------------

void GC9A01Driver::clear(
    uint32_t color)
{
    if (!initialized ||
        framebuffer == nullptr)
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Cannot clear GC9A01: display not initialized");

        return;
    }

    uint16_t rgb565 =
        rgb888_to_rgb565(color);

    uint8_t high_byte =
        static_cast<uint8_t>(
            rgb565 >> 8);

    uint8_t low_byte =
        static_cast<uint8_t>(
            rgb565 & 0xFF);

    const size_t pixel_count =
        static_cast<size_t>(width) *
        static_cast<size_t>(height);

    for (size_t index = 0;
         index < pixel_count;
         index++)
    {
        framebuffer[index * 2] =
            high_byte;

        framebuffer[(index * 2) + 1] =
            low_byte;
    }
}

// ------------------------------------------------------------
// ROTATION
// ------------------------------------------------------------

bool GC9A01Driver::set_rotation(
    TFTRotation requested_rotation)
{
    if (panel_handle == nullptr)
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Cannot set GC9A01 rotation: panel not initialized");

        return false;
    }

    int x_gap = 0;
    int y_gap = 0;
    bool swap_xy = false;
    bool mirror_x = false;
    bool mirror_y = false;

    /*
     * This GC9A01 module has a different physical mounting
     * orientation from the reference orientation used by the
     * generic MADCTL rotation mapping.
     *
     * The panel was physically validated using:
     *
     *     ROTATION_0   -> 0x88
     *     ROTATION_90  -> 0x68
     *     ROTATION_180 -> 0x48
     *     ROTATION_270 -> 0xA8
     *
     * BGR is supplied separately by panel_config and therefore
     * remains part of the MADCTL value automatically.
     */

    switch (requested_rotation)
    {
        case TFTRotation::ROTATION_0:

            swap_xy =
                false;

            mirror_x =
                false;

            mirror_y =
                true;

            break;

        case TFTRotation::ROTATION_90:

            swap_xy =
                true;

            mirror_x =
                true;

            mirror_y =
                false;

            break;

        case TFTRotation::ROTATION_180:

            swap_xy =
                false;

            mirror_x =
                true;

            mirror_y =
                false;

            break;

        case TFTRotation::ROTATION_270:

            swap_xy =
                true;

            mirror_x =
                false;

            mirror_y =
                true;

            break;

        default:

            LOG_ERROR(
                SYSTEM,
                HARDWARE,
                "Unsupported GC9A01 rotation");

            return false;
    }

    esp_err_t error =
        esp_lcd_panel_swap_xy(
            panel_handle,
            swap_xy);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to configure GC9A01 axis swap: %s",
            esp_err_to_name(error));

        return false;
    }

    error =
        esp_lcd_panel_mirror(
            panel_handle,
            mirror_x,
            mirror_y);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to configure GC9A01 mirror: %s",
            esp_err_to_name(error));

        return false;
    }

    error =
        esp_lcd_panel_set_gap(
            panel_handle,
            x_gap,
            y_gap);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to configure GC9A01 gap: %s",
            esp_err_to_name(error));

        return false;
    }

    rotation =
        requested_rotation;

    LOG_DEBUG(
        SYSTEM,
        HARDWARE,
        "GC9A01 rotation set to %u",
        static_cast<unsigned>(
            requested_rotation));

    return true;
}

// ------------------------------------------------------------
// FRAMEBUFFER PIXEL
// ------------------------------------------------------------

void GC9A01Driver::put_pixel(
    int16_t x,
    int16_t y,
    uint16_t color)
{
    if (framebuffer == nullptr)
    {
        return;
    }

    if (x < 0 ||
        y < 0 ||
        x >= static_cast<int16_t>(width) ||
        y >= static_cast<int16_t>(height))
    {
        return;
    }

    size_t pixel_index =
        (static_cast<size_t>(y) * width) +
        static_cast<size_t>(x);

    framebuffer[pixel_index * 2] =
        static_cast<uint8_t>(
            color >> 8);

    framebuffer[(pixel_index * 2) + 1] =
        static_cast<uint8_t>(
            color & 0xFF);
}

// ------------------------------------------------------------
// DRAW PIXEL
// ------------------------------------------------------------

bool GC9A01Driver::draw_pixel(
    uint16_t x,
    uint16_t y,
    uint32_t color)
{
    if (!initialized ||
        framebuffer == nullptr)
    {
        return false;
    }

    if (x >= width ||
        y >= height)
    {
        return false;
    }

    put_pixel(
        static_cast<int16_t>(x),
        static_cast<int16_t>(y),
        rgb888_to_rgb565(color));

    return true;
}

// ------------------------------------------------------------
// DRAW LINE
// ------------------------------------------------------------

void GC9A01Driver::draw_line(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    uint32_t color)
{
    if (!initialized ||
        framebuffer == nullptr)
    {
        return;
    }

    uint16_t rgb565 =
        rgb888_to_rgb565(color);

    int16_t dx =
        x1 - x0;

    int16_t dy =
        y1 - y0;

    int16_t sx =
        (dx >= 0) ? 1 : -1;

    int16_t sy =
        (dy >= 0) ? 1 : -1;

    dx =
        dx >= 0 ? dx : -dx;

    dy =
        dy >= 0 ? dy : -dy;

    int16_t error =
        dx - dy;

    while (true)
    {
        put_pixel(
            x0,
            y0,
            rgb565);

        if (x0 == x1 &&
            y0 == y1)
        {
            break;
        }

        int16_t error2 =
            error * 2;

        if (error2 > -dy)
        {
            error -= dy;
            x0 += sx;
        }

        if (error2 < dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}

// ------------------------------------------------------------
// DRAW RECTANGLE
// ------------------------------------------------------------

void GC9A01Driver::draw_rect(
    int16_t x,
    int16_t y,
    uint16_t rectangle_width,
    uint16_t rectangle_height,
    uint32_t color)
{
    if (rectangle_width == 0 ||
        rectangle_height == 0)
    {
        return;
    }

    draw_line(
        x,
        y,
        x + rectangle_width - 1,
        y,
        color);

    draw_line(
        x,
        y,
        x,
        y + rectangle_height - 1,
        color);

    draw_line(
        x + rectangle_width - 1,
        y,
        x + rectangle_width - 1,
        y + rectangle_height - 1,
        color);

    draw_line(
        x,
        y + rectangle_height - 1,
        x + rectangle_width - 1,
        y + rectangle_height - 1,
        color);
}

// ------------------------------------------------------------
// FILL RECTANGLE
// ------------------------------------------------------------

void GC9A01Driver::fill_rect(
    int16_t x,
    int16_t y,
    uint16_t rectangle_width,
    uint16_t rectangle_height,
    uint32_t color)
{
    if (!initialized ||
        framebuffer == nullptr)
    {
        return;
    }

    if (rectangle_width == 0 ||
        rectangle_height == 0)
    {
        return;
    }

    int16_t end_x =
        x + rectangle_width;

    int16_t end_y =
        y + rectangle_height;

    int16_t draw_x =
        x < 0 ? 0 : x;

    int16_t draw_y =
        y < 0 ? 0 : y;

    int16_t clipped_end_x =
        end_x > static_cast<int16_t>(width)
            ? static_cast<int16_t>(width)
            : end_x;

    int16_t clipped_end_y =
        end_y > static_cast<int16_t>(height)
            ? static_cast<int16_t>(height)
            : end_y;

    if (draw_x >= clipped_end_x ||
        draw_y >= clipped_end_y)
    {
        return;
    }

    uint16_t rgb565 =
        rgb888_to_rgb565(color);

    for (int16_t current_y = draw_y;
         current_y < clipped_end_y;
         current_y++)
    {
        size_t row_offset =
            static_cast<size_t>(current_y) *
            width *
            2;

        for (int16_t current_x = draw_x;
             current_x < clipped_end_x;
             current_x++)
        {
            size_t pixel_offset =
                row_offset +
                (static_cast<size_t>(current_x) * 2);

            framebuffer[pixel_offset] =
                static_cast<uint8_t>(
                    rgb565 >> 8);

            framebuffer[pixel_offset + 1] =
                static_cast<uint8_t>(
                    rgb565 & 0xFF);
        }
    }
}

// ------------------------------------------------------------
// DRAW CIRCLE
// ------------------------------------------------------------

void GC9A01Driver::draw_circle(
    int16_t center_x,
    int16_t center_y,
    uint16_t radius,
    uint32_t color)
{
    if (!initialized ||
        framebuffer == nullptr)
    {
        return;
    }

    uint16_t rgb565 =
        rgb888_to_rgb565(color);

    int16_t x = 0;

    int16_t y =
        static_cast<int16_t>(radius);

    int16_t decision =
        1 - y;

    while (x <= y)
    {
        put_pixel(
            center_x + x,
            center_y + y,
            rgb565);

        put_pixel(
            center_x - x,
            center_y + y,
            rgb565);

        put_pixel(
            center_x + x,
            center_y - y,
            rgb565);

        put_pixel(
            center_x - x,
            center_y - y,
            rgb565);

        put_pixel(
            center_x + y,
            center_y + x,
            rgb565);

        put_pixel(
            center_x - y,
            center_y + x,
            rgb565);

        put_pixel(
            center_x + y,
            center_y - x,
            rgb565);

        put_pixel(
            center_x - y,
            center_y - x,
            rgb565);

        x++;

        if (decision < 0)
        {
            decision +=
                (2 * x) + 1;
        }
        else
        {
            y--;

            decision +=
                (2 * (x - y)) + 1;
        }
    }
}

// ------------------------------------------------------------
// DRAW HORIZONTAL SPAN
// ------------------------------------------------------------

void GC9A01Driver::draw_horizontal_span(
    int16_t x0,
    int16_t x1,
    int16_t y,
    uint16_t color)
{
    if (framebuffer == nullptr)
    {
        return;
    }

    if (y < 0 ||
        y >= static_cast<int16_t>(height))
    {
        return;
    }

    if (x0 > x1)
    {
        int16_t temporary =
            x0;

        x0 = x1;
        x1 = temporary;
    }

    if (x1 < 0 ||
        x0 >= static_cast<int16_t>(width))
    {
        return;
    }

    if (x0 < 0)
    {
        x0 = 0;
    }

    if (x1 >= static_cast<int16_t>(width))
    {
        x1 =
            static_cast<int16_t>(width) - 1;
    }

    size_t row_offset =
        static_cast<size_t>(y) *
        width *
        2;

    for (int16_t x = x0;
         x <= x1;
         x++)
    {
        size_t pixel_offset =
            row_offset +
            (static_cast<size_t>(x) * 2);

        framebuffer[pixel_offset] =
            static_cast<uint8_t>(
                color >> 8);

        framebuffer[pixel_offset + 1] =
            static_cast<uint8_t>(
                color & 0xFF);
    }
}

// ------------------------------------------------------------
// FILL CIRCLE
// ------------------------------------------------------------

void GC9A01Driver::fill_circle(
    int16_t center_x,
    int16_t center_y,
    uint16_t radius,
    uint32_t color)
{
    if (!initialized ||
        framebuffer == nullptr)
    {
        return;
    }

    uint16_t rgb565 =
        rgb888_to_rgb565(color);

    int16_t x = 0;

    int16_t y =
        static_cast<int16_t>(radius);

    int16_t decision =
        1 - y;

    while (x <= y)
    {
        draw_horizontal_span(
            center_x - x,
            center_x + x,
            center_y + y,
            rgb565);

        draw_horizontal_span(
            center_x - x,
            center_x + x,
            center_y - y,
            rgb565);

        draw_horizontal_span(
            center_x - y,
            center_x + y,
            center_y + x,
            rgb565);

        draw_horizontal_span(
            center_x - y,
            center_x + y,
            center_y - x,
            rgb565);

        x++;

        if (decision < 0)
        {
            decision +=
                (2 * x) + 1;
        }
        else
        {
            y--;

            decision +=
                (2 * (x - y)) + 1;
        }
    }
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------

void GC9A01Driver::update()
{
    if (!initialized ||
        panel_handle == nullptr ||
        framebuffer == nullptr)
    {
        return;
    }

    for (uint16_t y = 0;
         y < height;
         y += TEST_STRIP_HEIGHT)
    {
        uint16_t strip_height =
            TEST_STRIP_HEIGHT;

        if ((y + strip_height) > height)
        {
            strip_height =
                height - y;
        }

        uint8_t *strip_buffer =
            &framebuffer[
                static_cast<size_t>(y) *
                width *
                2];

        esp_err_t error =
            esp_lcd_panel_draw_bitmap(
                panel_handle,
                0,
                y,
                width,
                y + strip_height,
                strip_buffer);

        if (error != ESP_OK)
        {
            LOG_ERROR(
                SYSTEM,
                HARDWARE,
                "Failed to update GC9A01 display: %s",
                esp_err_to_name(error));

            return;
        }
    }
}

// ------------------------------------------------------------
// STATE
// ------------------------------------------------------------

bool GC9A01Driver::is_initialized() const
{
    return initialized;
}

uint16_t GC9A01Driver::get_width() const
{
    return width;
}

uint16_t GC9A01Driver::get_height() const
{
    return height;
}