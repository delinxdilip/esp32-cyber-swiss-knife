#pragma once

#include <cstdint>

/**
 * Supported display controllers.
 *
 * Add new controllers here as they are implemented.
 * The display manager will use this value to select
 * the appropriate driver.
 */
enum class TFTController : uint8_t
{
    GC9A01 = 0
};

/**
 * Physical display shape.
 */
enum class TFTShape : uint8_t
{
    ROUND = 0,
    RECTANGLE
};

/**
 * Display rotation.
 */
enum class TFTRotation : uint8_t
{
    ROTATION_0 = 0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270
};

/**
 * Generic TFT configuration.
 *
 * This structure intentionally contains only properties
 * that are common to supported display controllers.
 *
 * Controller-specific hardware details belong inside
 * the individual display driver implementation.
 */
struct TFTConfig
{
    bool enabled;

    TFTController controller;
    TFTShape shape;

    uint16_t width;
    uint16_t height;

    TFTRotation rotation;

    bool touch;

    uint32_t color;
};
