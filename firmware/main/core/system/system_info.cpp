#include "system_info.h"

#include "core/logging/logger.h"

#include "esp_chip_info.h"
#include "esp_clk_tree.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_mac.h"
#include "esp_system.h"

void SystemInfo::print()
{
    // ----------------------------------------
    // Chip information
    // ----------------------------------------

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    LOG_INFO(
        SYSTEM,
        "Chip cores: %d",
        chip_info.cores);

    LOG_INFO(
        SYSTEM,
        "Chip revision: %d",
        chip_info.revision);

    // ----------------------------------------
    // CPU frequency
    // ----------------------------------------

    uint32_t cpu_freq = 0;

    if (esp_clk_tree_src_get_freq_hz(
            SOC_MOD_CLK_CPU,
            ESP_CLK_TREE_SRC_FREQ_PRECISION_EXACT,
            &cpu_freq) == ESP_OK)
    {
        LOG_INFO(
            SYSTEM,
            "CPU frequency: %lu MHz",
            (unsigned long)(cpu_freq / 1000000));
    }
    else
    {
        LOG_WARN(
            SYSTEM,
            "CPU frequency: unavailable");
    }

    // ----------------------------------------
    // Flash
    // ----------------------------------------

    uint32_t flash_size = 0;

    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK)
    {
        LOG_INFO(
            SYSTEM,
            "Flash size: %lu MB",
            (unsigned long)(flash_size / (1024 * 1024)));
    }
    else
    {
        LOG_WARN(
            SYSTEM,
            "Flash size: unavailable");
    }

    // ----------------------------------------
    // Internal RAM
    // ----------------------------------------

    LOG_INFO(
        SYSTEM,
        "Free heap: %lu KB",
        (unsigned long)(
            heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024));

    // ----------------------------------------
    // PSRAM
    // ----------------------------------------

    size_t psram_free =
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    size_t psram_total =
        heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

    LOG_INFO(
        SYSTEM,
        "PSRAM total: %lu KB",
        (unsigned long)(psram_total / 1024));

    LOG_INFO(
        SYSTEM,
        "Free PSRAM: %lu KB",
        (unsigned long)(psram_free / 1024));

    // ----------------------------------------
    // ESP-IDF
    // ----------------------------------------

    LOG_INFO(
        SYSTEM,
        "ESP-IDF version: %s",
        esp_get_idf_version());

    // ----------------------------------------
    // MAC address
    // ----------------------------------------

    uint8_t mac[6];

    if (esp_read_mac(
            mac,
            ESP_MAC_WIFI_STA) == ESP_OK)
    {
        LOG_INFO(
            SYSTEM,
            "Wi-Fi MAC: %02X:%02X:%02X:%02X:%02X:%02X",
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);
    }
    else
    {
        LOG_WARN(
            SYSTEM,
            "Wi-Fi MAC: unavailable");
    }

    // ----------------------------------------
    // Reset reason
    // ----------------------------------------

    LOG_INFO(
        SYSTEM,
        "Reset reason: %d",
        esp_reset_reason());
}