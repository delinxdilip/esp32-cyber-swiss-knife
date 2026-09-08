#pragma once

#include <stddef.h>
#include <stdint.h>

struct SystemInfoData
{
    // Device
    char device_name[33];
    char chip_name[32];
    uint32_t chip_revision;
    uint32_t cpu_cores;
    uint32_t cpu_frequency_mhz;
    char esp_idf_version[32];
    char firmware_version[16];
    char build_date[32];
    uint64_t uptime_seconds;
    char reset_reason[32];

    // Memory
    uint32_t free_internal_ram;
    uint32_t total_internal_ram;
    uint32_t used_internal_ram;
    uint32_t minimum_free_heap;
    uint32_t psram_total;
    uint32_t psram_free;

    // Flash
    uint32_t flash_size;
    uint32_t flash_speed_mhz;
    char flash_mode[16];

    uint32_t application_partition_size;
    uint32_t spiffs_partition_size;
    uint32_t spiffs_used;
    uint32_t spiffs_free;

    // Hardware / Health
    float temperature_celsius;
    uint32_t hardware_cpu_cores;
    uint32_t hardware_chip_revision;

    char wifi_state[16];
    char bluetooth_state[16];
    char ap_state[16];

    uint32_t connected_clients;
    float cpu_load_percent;
    uint32_t task_count;

    // Network
    char ap_status[16];
    char ap_ssid[33];
    char ap_ip_address[16];
    uint32_t ap_channel;
    uint32_t ap_clients;

    char mac_address[18];
    char wifi_status[16];

    // Storage
    uint32_t storage_total;
    uint32_t storage_used;
    uint32_t storage_free;
    uint32_t storage_files;

    uint32_t logs_bytes;
    uint32_t web_ui_bytes;
    uint32_t configuration_bytes;

    // Firmware
    char firmware_name[33];
    char firmware_version_detail[16];
    char firmware_build_date[32];
    char firmware_build_time[16];
    char firmware_target[32];
    char firmware_framework[32];
};

class SystemInfo
{
public:
    static void print();

    static bool get(
        SystemInfoData &info);

private:
    static void set_string(
        char *destination,
        size_t destination_size,
        const char *source);

    static const char *get_reset_reason_string(
        int reason);

    static void get_flash_info(
        SystemInfoData &info);

    static void get_partition_info(
        SystemInfoData &info);

    static void get_storage_info(
        SystemInfoData &info);

    static void get_network_info(
        SystemInfoData &info);

    static void get_runtime_info(
        SystemInfoData &info);
};