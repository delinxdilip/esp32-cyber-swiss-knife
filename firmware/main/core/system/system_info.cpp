#include "system_info.h"

#include <cstdio>
#include <cstring>

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_spiffs.h"
#include "esp_mac.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "core/config/config_manager.h"
#include "core/logging/logger.h"
#include "core/monitoring/temperature/temperature_monitor.h"
#include "core/monitoring/uptime/uptime_monitor.h"

namespace
{
    constexpr const char *FIRMWARE_NAME =
        "CyberSwissKnife";

    constexpr const char *FIRMWARE_VERSION =
        "0.1.0";

    constexpr const char *FIRMWARE_TARGET =
        "ESP32-S3";

    constexpr const char *FIRMWARE_FRAMEWORK =
        "ESP-IDF";
}

void SystemInfo::set_string(
    char *destination,
    size_t destination_size,
    const char *source)
{
    if (destination == nullptr ||
        destination_size == 0)
    {
        return;
    }

    if (source == nullptr)
    {
        destination[0] = '\0';
        return;
    }

    std::strncpy(
        destination,
        source,
        destination_size - 1);

    destination[destination_size - 1] = '\0';
}

const char *SystemInfo::get_reset_reason_string(
    int reason)
{
    switch (reason)
    {
        case ESP_RST_UNKNOWN:
            return "Unknown";

        case ESP_RST_POWERON:
            return "Power-on";

        case ESP_RST_EXT:
            return "External reset";

        case ESP_RST_SW:
            return "Software reset";

        case ESP_RST_PANIC:
            return "Panic";

        case ESP_RST_INT_WDT:
            return "Interrupt watchdog";

        case ESP_RST_TASK_WDT:
            return "Task watchdog";

        case ESP_RST_WDT:
            return "Other watchdog";

        case ESP_RST_DEEPSLEEP:
            return "Deep sleep";

        case ESP_RST_BROWNOUT:
            return "Brownout";

        case ESP_RST_SDIO:
            return "SDIO";

        default:
            return "Unknown";
    }
}

void SystemInfo::get_flash_info(
    SystemInfoData &info)
{
    uint32_t flash_size = 0;

    if (esp_flash_get_size(
            nullptr,
            &flash_size) == ESP_OK)
    {
        info.flash_size =
            flash_size;
    }

    /*
     * ESP-IDF 6.x does not expose the previous
     * esp_flash_get_speed() API we attempted to use.
     *
     * The configured flash frequency is available
     * through the build configuration.
     */
#if CONFIG_ESPTOOLPY_FLASHFREQ_80M
    info.flash_speed_mhz = 80;
#elif CONFIG_ESPTOOLPY_FLASHFREQ_40M
    info.flash_speed_mhz = 40;
#elif CONFIG_ESPTOOLPY_FLASHFREQ_26M
    info.flash_speed_mhz = 26;
#elif CONFIG_ESPTOOLPY_FLASHFREQ_20M
    info.flash_speed_mhz = 20;
#else
    info.flash_speed_mhz = 0;
#endif

#if CONFIG_ESPTOOLPY_FLASHMODE_QIO
    set_string(
        info.flash_mode,
        sizeof(info.flash_mode),
        "QIO");
#elif CONFIG_ESPTOOLPY_FLASHMODE_QOUT
    set_string(
        info.flash_mode,
        sizeof(info.flash_mode),
        "QOUT");
#elif CONFIG_ESPTOOLPY_FLASHMODE_DIO
    set_string(
        info.flash_mode,
        sizeof(info.flash_mode),
        "DIO");
#elif CONFIG_ESPTOOLPY_FLASHMODE_DOUT
    set_string(
        info.flash_mode,
        sizeof(info.flash_mode),
        "DOUT");
#else
    set_string(
        info.flash_mode,
        sizeof(info.flash_mode),
        "Unknown");
#endif
}

void SystemInfo::get_partition_info(
    SystemInfoData &info)
{
    const esp_partition_t *app_partition =
        esp_ota_get_running_partition();

    if (app_partition != nullptr)
    {
        info.application_partition_size =
            app_partition->size;
    }

    const esp_partition_t *spiffs_partition =
        esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA,
            ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
            nullptr);

    if (spiffs_partition != nullptr)
    {
        info.spiffs_partition_size =
            spiffs_partition->size;
    }
}

void SystemInfo::get_storage_info(
    SystemInfoData &info)
{
    size_t total = 0;
    size_t used = 0;

    if (esp_spiffs_info(
            nullptr,
            &total,
            &used) == ESP_OK)
    {
        info.storage_total =
            static_cast<uint32_t>(total);

        info.storage_used =
            static_cast<uint32_t>(used);

        info.storage_free =
            static_cast<uint32_t>(
                total > used
                    ? total - used
                    : 0);

        info.spiffs_used =
            static_cast<uint32_t>(used);

        info.spiffs_free =
            static_cast<uint32_t>(
                total > used
                    ? total - used
                    : 0);
    }

    /*
     * Detailed file accounting will be implemented
     * by the storage/logging subsystem.
     */
    info.storage_files = 0;
    info.logs_bytes = 0;

    /*
     * Until directory-level accounting exists,
     * the currently used filesystem space represents
     * the Web UI footprint.
     */
    info.web_ui_bytes =
        info.storage_used;

    info.configuration_bytes =
        sizeof(APConfig);
}

void SystemInfo::get_network_info(
    SystemInfoData &info)
{
    wifi_config_t ap_config{};

    if (esp_wifi_get_config(
            WIFI_IF_AP,
            &ap_config) == ESP_OK)
    {
        set_string(
            info.ap_ssid,
            sizeof(info.ap_ssid),
            reinterpret_cast<const char *>(
                ap_config.ap.ssid));

        info.ap_channel =
            ap_config.ap.channel;
    }

    uint8_t mac[6] = {};

    if (esp_read_mac(
            mac,
            ESP_MAC_WIFI_SOFTAP) == ESP_OK)
    {
        std::snprintf(
            info.mac_address,
            sizeof(info.mac_address),
            "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);
    }

    esp_netif_t *ap_netif =
        esp_netif_get_handle_from_ifkey(
            "WIFI_AP_DEF");

    if (ap_netif != nullptr)
    {
        esp_netif_ip_info_t ip_info{};

        if (esp_netif_get_ip_info(
                ap_netif,
                &ip_info) == ESP_OK)
        {
            std::snprintf(
                info.ap_ip_address,
                sizeof(info.ap_ip_address),
                IPSTR,
                IP2STR(&ip_info.ip));
        }
    }

    wifi_mode_t mode =
        WIFI_MODE_NULL;

    if (esp_wifi_get_mode(
            &mode) == ESP_OK)
    {
        switch (mode)
        {
            case WIFI_MODE_AP:
                set_string(
                    info.wifi_status,
                    sizeof(info.wifi_status),
                    "AP");
                break;

            case WIFI_MODE_STA:
                set_string(
                    info.wifi_status,
                    sizeof(info.wifi_status),
                    "Station");
                break;

            case WIFI_MODE_APSTA:
                set_string(
                    info.wifi_status,
                    sizeof(info.wifi_status),
                    "AP + STA");
                break;

            default:
                set_string(
                    info.wifi_status,
                    sizeof(info.wifi_status),
                    "Inactive");
                break;
        }
    }

    set_string(
        info.wifi_state,
        sizeof(info.wifi_state),
        "Running");

    set_string(
        info.ap_status,
        sizeof(info.ap_status),
        "Running");

    set_string(
        info.ap_state,
        sizeof(info.ap_state),
        "Running");

    wifi_sta_list_t station_list{};

    if (esp_wifi_ap_get_sta_list(
            &station_list) == ESP_OK)
    {
        info.connected_clients =
            station_list.num;

        info.ap_clients =
            station_list.num;
    }
}

void SystemInfo::get_runtime_info(
    SystemInfoData &info)
{
    info.free_internal_ram =
        heap_caps_get_free_size(
            MALLOC_CAP_INTERNAL);

    info.total_internal_ram =
        heap_caps_get_total_size(
            MALLOC_CAP_INTERNAL);

    info.used_internal_ram =
        info.total_internal_ram >
                info.free_internal_ram
            ? info.total_internal_ram -
                info.free_internal_ram
            : 0;

    info.minimum_free_heap =
        esp_get_minimum_free_heap_size();

    info.psram_total =
        heap_caps_get_total_size(
            MALLOC_CAP_SPIRAM);

    info.psram_free =
        heap_caps_get_free_size(
            MALLOC_CAP_SPIRAM);

    info.task_count =
        uxTaskGetNumberOfTasks();

    /*
     * CPU load requires FreeRTOS runtime statistics.
     *
     * We intentionally don't fake this value.
     * It will remain 0 until proper runtime
     * accounting is enabled.
     */
    info.cpu_load_percent = 0.0f;
}

bool SystemInfo::get(
    SystemInfoData &info)
{
    std::memset(
        &info,
        0,
        sizeof(info));

    set_string(
        info.device_name,
        sizeof(info.device_name),
        FIRMWARE_NAME);

    set_string(
        info.firmware_name,
        sizeof(info.firmware_name),
        FIRMWARE_NAME);

    set_string(
        info.firmware_version,
        sizeof(info.firmware_version),
        FIRMWARE_VERSION);

    set_string(
        info.firmware_version_detail,
        sizeof(info.firmware_version_detail),
        FIRMWARE_VERSION);

    set_string(
        info.firmware_target,
        sizeof(info.firmware_target),
        FIRMWARE_TARGET);

    set_string(
        info.firmware_framework,
        sizeof(info.firmware_framework),
        FIRMWARE_FRAMEWORK);

    set_string(
        info.build_date,
        sizeof(info.build_date),
        __DATE__);

    set_string(
        info.firmware_build_date,
        sizeof(info.firmware_build_date),
        __DATE__);

    set_string(
        info.firmware_build_time,
        sizeof(info.firmware_build_time),
        __TIME__);

    esp_chip_info_t chip_info{};

    esp_chip_info(
        &chip_info);

    set_string(
        info.chip_name,
        sizeof(info.chip_name),
        "ESP32-S3");

    info.chip_revision =
        chip_info.revision;

    info.hardware_chip_revision =
        chip_info.revision;

    info.cpu_cores =
        chip_info.cores;

    info.hardware_cpu_cores =
        chip_info.cores;

    /*
     * ESP32-S3 default CPU frequency for the
     * current project configuration.
     *
     * We avoid the unavailable clock-tree API here.
     */
#if CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ
    info.cpu_frequency_mhz =
        CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
#else
    info.cpu_frequency_mhz = 160;
#endif

    set_string(
        info.esp_idf_version,
        sizeof(info.esp_idf_version),
        esp_get_idf_version());

    info.uptime_seconds =
        UptimeMonitor::get_seconds();

    set_string(
        info.reset_reason,
        sizeof(info.reset_reason),
        get_reset_reason_string(
            static_cast<int>(
                esp_reset_reason())));

    get_runtime_info(info);

    if (TemperatureMonitor::init())
    {
        float temperature = 0.0f;

        if (TemperatureMonitor::get_celsius(
                temperature))
        {
            info.temperature_celsius =
                temperature;
        }
    }

    get_flash_info(info);

    get_partition_info(info);

    get_storage_info(info);

    get_network_info(info);

    set_string(
        info.bluetooth_state,
        sizeof(info.bluetooth_state),
        "Not initialized");

    return true;
}

void SystemInfo::print()
{
    SystemInfoData info;

    if (!get(info))
    {
        LOG_ERROR(
            SYSTEM,
            "Failed to collect system information");

        return;
    }

    LOG_INFO(
        SYSTEM,
        "Chip: %s Rev %lu",
        info.chip_name,
        static_cast<unsigned long>(
            info.chip_revision));

    LOG_INFO(
        SYSTEM,
        "CPU: %lu cores @ %lu MHz",
        static_cast<unsigned long>(
            info.cpu_cores),
        static_cast<unsigned long>(
            info.cpu_frequency_mhz));

    LOG_INFO(
        SYSTEM,
        "ESP-IDF: %s",
        info.esp_idf_version);

    LOG_INFO(
        SYSTEM,
        "Firmware: %s v%s",
        info.firmware_name,
        info.firmware_version);

    LOG_INFO(
        SYSTEM,
        "Build: %s %s",
        info.firmware_build_date,
        info.firmware_build_time);

    LOG_INFO(
        SYSTEM,
        "Free internal RAM: %lu KB",
        static_cast<unsigned long>(
            info.free_internal_ram / 1024));

    LOG_INFO(
        SYSTEM,
        "PSRAM: %lu KB total / %lu KB free",
        static_cast<unsigned long>(
            info.psram_total / 1024),
        static_cast<unsigned long>(
            info.psram_free / 1024));

    LOG_INFO(
        SYSTEM,
        "Flash: %lu MB @ %lu MHz %s",
        static_cast<unsigned long>(
            info.flash_size /
            (1024 * 1024)),
        static_cast<unsigned long>(
            info.flash_speed_mhz),
        info.flash_mode);

    LOG_INFO(
        SYSTEM,
        "Temperature: %.2f C",
        info.temperature_celsius);

    LOG_INFO(
        SYSTEM,
        "Tasks: %lu",
        static_cast<unsigned long>(
            info.task_count));

    LOG_INFO(
        SYSTEM,
        "CPU load: %.1f%%",
        info.cpu_load_percent);

    LOG_INFO(
        SYSTEM,
        "AP: %s / %s / channel %lu / clients %lu",
        info.ap_status,
        info.ap_ssid,
        static_cast<unsigned long>(
            info.ap_channel),
        static_cast<unsigned long>(
            info.ap_clients));

    LOG_INFO(
        SYSTEM,
        "Storage: %lu KB used / %lu KB free",
        static_cast<unsigned long>(
            info.storage_used / 1024),
        static_cast<unsigned long>(
            info.storage_free / 1024));
}