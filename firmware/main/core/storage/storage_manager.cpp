#include "storage_manager.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

namespace
{
    constexpr const char *TAG = "StorageManager";

    constexpr const char *FILESYSTEM_LABEL = "spiffs";
    constexpr const char *FILESYSTEM_PATH = "/spiffs";
}

bool StorageManager::initialized = false;


// ============================================================
// INITIALIZE
// ============================================================

bool StorageManager::initialize()
{
    if (initialized)
    {
        return true;
    }

    esp_vfs_spiffs_conf_t config =
    {
        .base_path = FILESYSTEM_PATH,
        .partition_label = FILESYSTEM_LABEL,
        .max_files = 10,
        .format_if_mount_failed = false
    };

    esp_err_t err =
        esp_vfs_spiffs_register(&config);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to initialize SPIFFS: %s",
            esp_err_to_name(err));

        return false;
    }

    initialized = true;

    ESP_LOGI(
        TAG,
        "SPIFFS initialized");

    ESP_LOGI(
        TAG,
        "Total: %llu bytes",
        get_total_space());

    ESP_LOGI(
        TAG,
        "Used: %llu bytes",
        get_used_space());

    ESP_LOGI(
        TAG,
        "Free: %llu bytes",
        get_free_space());

    return true;
}


// ============================================================
// TOTAL SPACE
// ============================================================

uint64_t StorageManager::get_total_space()
{
    if (!initialized)
    {
        return 0;
    }

    size_t total = 0;
    size_t used = 0;

    esp_err_t err =
        esp_spiffs_info(
            FILESYSTEM_LABEL,
            &total,
            &used);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to get SPIFFS information: %s",
            esp_err_to_name(err));

        return 0;
    }

    return static_cast<uint64_t>(total);
}


// ============================================================
// USED SPACE
// ============================================================

uint64_t StorageManager::get_used_space()
{
    if (!initialized)
    {
        return 0;
    }

    size_t total = 0;
    size_t used = 0;

    esp_err_t err =
        esp_spiffs_info(
            FILESYSTEM_LABEL,
            &total,
            &used);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to get SPIFFS information: %s",
            esp_err_to_name(err));

        return 0;
    }

    return static_cast<uint64_t>(used);
}


// ============================================================
// FREE SPACE
// ============================================================

uint64_t StorageManager::get_free_space()
{
    if (!initialized)
    {
        return 0;
    }

    const uint64_t total =
        get_total_space();

    const uint64_t used =
        get_used_space();

    if (used >= total)
    {
        return 0;
    }

    return total - used;
}


// ============================================================
// BASE PATH
// ============================================================

const char *StorageManager::get_base_path()
{
    return FILESYSTEM_PATH;
}


// ============================================================
// INITIALIZATION STATUS
// ============================================================

bool StorageManager::is_initialized()
{
    return initialized;
}