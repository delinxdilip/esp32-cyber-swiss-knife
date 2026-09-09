#include "display_manager.h"

#include "core/config/config_manager.h"

#include "core/logging/logger.h"

#include "drivers/gc9a01/gc9a01_driver.h"

DisplayDriver *DisplayManager::driver = nullptr;

bool DisplayManager::initialized = false;

bool DisplayManager::initialize()
{
    if (initialized)
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Display manager is already initialized");

        return true;
    }

    const TFTConfig &config =
        ConfigManager::get_tft_config();

    if (!config.enabled)
    {
        LOG_INFO(
            SYSTEM,
            HARDWARE,
            "TFT display is disabled in configuration");

        initialized = true;

        return true;
    }

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "Initializing display controller: %u",
        static_cast<unsigned>(config.controller));

    if (!create_driver())
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to create display driver");

        return false;
    }

    if (!driver->initialize(config))
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to initialize display driver");

        destroy_driver();

        return false;
    }

    initialized = true;

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "Display manager initialized successfully");

    return true;
}

void DisplayManager::deinitialize()
{
    if (!initialized && driver == nullptr)
    {
        return;
    }

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "Deinitializing display manager");

    destroy_driver();

    initialized = false;
}

bool DisplayManager::create_driver()
{
    if (driver != nullptr)
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Display driver already exists");

        return true;
    }

    const TFTConfig &config =
        ConfigManager::get_tft_config();

    switch (config.controller)
    {
        case TFTController::GC9A01:

            LOG_DEBUG(
                SYSTEM,
                HARDWARE,
                "Selecting GC9A01 display driver");

            driver = new GC9A01Driver();

            break;

        default:

            LOG_ERROR(
                SYSTEM,
                HARDWARE,
                "Unsupported display controller: %u",
                static_cast<unsigned>(config.controller));

            driver = nullptr;

            return false;
    }

    if (driver == nullptr)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to allocate display driver");

        return false;
    }

    return true;
}

void DisplayManager::destroy_driver()
{
    if (driver == nullptr)
    {
        return;
    }

    if (driver->is_initialized())
    {
        driver->deinitialize();
    }

    delete driver;

    driver = nullptr;
}

void DisplayManager::clear(
    uint32_t color)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Cannot clear display: no active driver");

        return;
    }

    driver->clear(
        color);
}

bool DisplayManager::set_rotation(
    TFTRotation rotation)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Cannot set display rotation: no active driver");

        return false;
    }

    return driver->set_rotation(
        rotation);
}

bool DisplayManager::draw_pixel(
    uint16_t x,
    uint16_t y,
    uint32_t color)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Cannot draw pixel: no active display driver");

        return false;
    }

    return driver->draw_pixel(
        x,
        y,
        color);
}

// ------------------------------------------------------------
// DRAW LINE
// ------------------------------------------------------------

void DisplayManager::draw_line(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    uint32_t color)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        return;
    }

    driver->draw_line(
        x0,
        y0,
        x1,
        y1,
        color);
}

// ------------------------------------------------------------
// DRAW RECTANGLE
// ------------------------------------------------------------

void DisplayManager::draw_rect(
    int16_t x,
    int16_t y,
    uint16_t width,
    uint16_t height,
    uint32_t color)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        return;
    }

    driver->draw_rect(
        x,
        y,
        width,
        height,
        color);
}

// ------------------------------------------------------------
// FILL RECTANGLE
// ------------------------------------------------------------

void DisplayManager::fill_rect(
    int16_t x,
    int16_t y,
    uint16_t width,
    uint16_t height,
    uint32_t color)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        return;
    }

    driver->fill_rect(
        x,
        y,
        width,
        height,
        color);
}

// ------------------------------------------------------------
// DRAW CIRCLE
// ------------------------------------------------------------

void DisplayManager::draw_circle(
    int16_t center_x,
    int16_t center_y,
    uint16_t radius,
    uint32_t color)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        return;
    }

    driver->draw_circle(
        center_x,
        center_y,
        radius,
        color);
}

// ------------------------------------------------------------
// FILL CIRCLE
// ------------------------------------------------------------

void DisplayManager::fill_circle(
    int16_t center_x,
    int16_t center_y,
    uint16_t radius,
    uint32_t color)
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        return;
    }

    driver->fill_circle(
        center_x,
        center_y,
        radius,
        color);
}

void DisplayManager::update()
{
    if (driver == nullptr ||
        !driver->is_initialized())
    {
        return;
    }

    driver->update();
}

bool DisplayManager::is_initialized()
{
    return initialized;
}

bool DisplayManager::has_driver()
{
    return driver != nullptr;
}

DisplayDriver *DisplayManager::get_driver()
{
    return driver;
}