#include "gc9a01_display.h"

#include <stdint.h>

#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_gc9a01.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "GC9A01";

// ------------------------------------------------------------
// Pin configuration
// ------------------------------------------------------------

#define LCD_HOST       SPI2_HOST

#define LCD_SCLK_GPIO  GPIO_NUM_15
#define LCD_MOSI_GPIO  GPIO_NUM_16
#define LCD_DC_GPIO    GPIO_NUM_7
#define LCD_CS_GPIO    GPIO_NUM_6
#define LCD_RST_GPIO   GPIO_NUM_5

#define LCD_H_RES      240
#define LCD_V_RES      240

// Keep the first test small.
// 240 x 20 x 2 bytes = 9.6 KB.
#define TEST_STRIP_HEIGHT 20

static esp_lcd_panel_handle_t panel_handle = NULL;


// ------------------------------------------------------------
// Display initialization
// ------------------------------------------------------------

esp_err_t gc9a01_display_init(void)
{
    ESP_LOGI(TAG, "Initializing GC9A01");

    // --------------------------------------------------------
    // SPI bus
    // --------------------------------------------------------

    spi_bus_config_t bus_config = {};

    bus_config.sclk_io_num = LCD_SCLK_GPIO;
    bus_config.mosi_io_num = LCD_MOSI_GPIO;
    bus_config.miso_io_num = -1;
    bus_config.quadwp_io_num = -1;
    bus_config.quadhd_io_num = -1;

    bus_config.max_transfer_sz =
        LCD_H_RES * TEST_STRIP_HEIGHT * sizeof(uint16_t);

    ESP_LOGI(TAG, "Initializing SPI bus");

    ESP_ERROR_CHECK(
        spi_bus_initialize(
            LCD_HOST,
            &bus_config,
            SPI_DMA_CH_AUTO
        )
    );


    // --------------------------------------------------------
    // SPI panel IO
    // --------------------------------------------------------

    esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_spi_config_t io_config = {};

    io_config.dc_gpio_num = LCD_DC_GPIO;
    io_config.cs_gpio_num = LCD_CS_GPIO;

    // Start conservatively.
    io_config.pclk_hz = 20 * 1000 * 1000;

    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;

    io_config.spi_mode = 0;
    io_config.trans_queue_depth = 10;

    ESP_LOGI(TAG, "Creating SPI panel IO");

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_spi(
            (esp_lcd_spi_bus_handle_t)LCD_HOST,
            &io_config,
            &io_handle
        )
    );


    // --------------------------------------------------------
    // GC9A01 panel
    // --------------------------------------------------------

    esp_lcd_panel_dev_config_t panel_config = {};

    panel_config.reset_gpio_num = LCD_RST_GPIO;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_config.bits_per_pixel = 16;

    ESP_LOGI(TAG, "Creating GC9A01 panel");

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_gc9a01(
            io_handle,
            &panel_config,
            &panel_handle
        )
    );


    // --------------------------------------------------------
    // Hardware reset
    // --------------------------------------------------------

    ESP_LOGI(TAG, "Resetting display");

    ESP_ERROR_CHECK(
        esp_lcd_panel_reset(panel_handle)
    );


    // --------------------------------------------------------
    // Initialize controller
    // --------------------------------------------------------

    ESP_LOGI(TAG, "Initializing controller");

    ESP_ERROR_CHECK(
        esp_lcd_panel_init(panel_handle)
    );


    // --------------------------------------------------------
    // Turn display on
    // --------------------------------------------------------

    ESP_LOGI(TAG, "Turning display on");

    ESP_ERROR_CHECK(
        esp_lcd_panel_disp_on_off(
            panel_handle,
            true
        )
    );

    ESP_LOGI(TAG, "GC9A01 initialization complete");

    return ESP_OK;
}


// ------------------------------------------------------------
// Simple color test
// ------------------------------------------------------------

esp_err_t gc9a01_display_test(void)
{
    if (panel_handle == NULL) {
        ESP_LOGE(TAG, "Display is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Starting display test");

    static uint16_t buffer[
        LCD_H_RES * TEST_STRIP_HEIGHT
    ];

    const uint16_t colors[] = {
        0xF800, // RED
        0x07E0, // GREEN
        0x001F, // BLUE
        0xFFFF, // WHITE
        0x0000  // BLACK
    };

    const int color_count =
        sizeof(colors) / sizeof(colors[0]);

    for (int color_index = 0;
         color_index < color_count;
         color_index++)
    {
        uint16_t color = colors[color_index];

        for (size_t i = 0;
             i < sizeof(buffer) / sizeof(buffer[0]);
             i++)
        {
            buffer[i] = color;
        }

        for (int y = 0;
             y < LCD_V_RES;
             y += TEST_STRIP_HEIGHT)
        {
            ESP_ERROR_CHECK(
                esp_lcd_panel_draw_bitmap(
                    panel_handle,
                    0,
                    y,
                    LCD_H_RES,
                    y + TEST_STRIP_HEIGHT,
                    buffer
                )
            );
        }

        ESP_LOGI(
            TAG,
            "Color %d/%d",
            color_index + 1,
            color_count
        );

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }

    ESP_LOGI(TAG, "Display test complete");

    return ESP_OK;
}