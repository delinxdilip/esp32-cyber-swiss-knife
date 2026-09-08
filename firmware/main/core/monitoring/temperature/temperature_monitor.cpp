#include "temperature_monitor.h"

#include "core/logging/logger.h"

#include "driver/temperature_sensor.h"
#include "esp_err.h"

namespace
{
    temperature_sensor_handle_t temperature_sensor = nullptr;
    bool initialized = false;
}

bool TemperatureMonitor::init()
{
    if (initialized)
    {
        LOG_DEBUG(
            SYSTEM,
            SYSTEM,
            "Temperature monitor already initialized");

        return true;
    }

    temperature_sensor_config_t config =
        TEMPERATURE_SENSOR_CONFIG_DEFAULT(20, 50);

    esp_err_t err =
        temperature_sensor_install(
            &config,
            &temperature_sensor);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            SYSTEM,
            "Temperature sensor install failed: %s",
            esp_err_to_name(err));

        return false;
    }

    err =
        temperature_sensor_enable(
            temperature_sensor);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            SYSTEM,
            "Temperature sensor enable failed: %s",
            esp_err_to_name(err));

        temperature_sensor = nullptr;

        return false;
    }

    initialized = true;

    LOG_INFO(
        SYSTEM,
        SYSTEM,
        "Temperature monitor initialized");

    return true;
}

bool TemperatureMonitor::get_celsius(float &temperature)
{
    if (!initialized || temperature_sensor == nullptr)
    {
        LOG_WARN(
            SYSTEM,
            SYSTEM,
            "Temperature monitor is not initialized");

        return false;
    }

    esp_err_t err =
        temperature_sensor_get_celsius(
            temperature_sensor,
            &temperature);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            SYSTEM,
            "Temperature reading failed: %s",
            esp_err_to_name(err));

        return false;
    }

    return true;
}