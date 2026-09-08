#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>


enum class LogType : uint8_t
{
    SYSTEM = 0,
    ACTIVITY
};


enum class LogLevel : uint8_t
{
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};


struct LogEntry
{
    uint64_t timestamp;
    LogLevel level;
    char source[32];
    char message[128];
};


struct LogStorageStats
{
    uint64_t total;
    uint64_t used;
    uint64_t remaining;
    uint64_t entry_count;
    bool full;
};


struct LogReadHandle
{
    FILE *file;
    LogType type;
    bool active;
};


class LogManager
{
public:

    static bool initialize();

    static LogStorageStats get_system_log_stats();
    static LogStorageStats get_activity_log_stats();


    // ========================================================
    // STREAMING LOG API
    // ========================================================

    static bool open_system_logs(
        LogReadHandle &handle);

    static bool open_activity_logs(
        LogReadHandle &handle);

    static size_t read_log_chunk(
        LogReadHandle &handle,
        char *buffer,
        size_t buffer_size);

    static void close_logs(
        LogReadHandle &handle);


    // ========================================================
    // TFT LOG API
    // ========================================================

    static bool view_system_logs_tft(
        uint32_t page,
        uint32_t entries_per_page,
        char *buffer,
        size_t buffer_size);

    static bool view_activity_logs_tft(
        uint32_t page,
        uint32_t entries_per_page,
        char *buffer,
        size_t buffer_size);


    // ========================================================
    // LOG MANAGEMENT
    // ========================================================

    static bool delete_system_logs();
    static bool delete_activity_logs();

    static bool clear_old_system_logs();
    static bool clear_old_activity_logs();

    static bool check_storage();
    static bool handle_auto_clear();


    // ========================================================
    // LOG CREATION
    // ========================================================

    static bool add_system_log(
        LogLevel level,
        const char *source,
        const char *message);

    static bool add_activity_log(
        LogLevel level,
        const char *source,
        const char *message);


    static bool is_initialized();


private:

    // ========================================================
    // LOG CREATION
    // ========================================================

    static bool add_log(
        LogType type,
        LogLevel level,
        const char *source,
        const char *message);


    // ========================================================
    // LOG READING
    // ========================================================

    static bool open_logs(
        LogType type,
        LogReadHandle &handle);

    static bool read_logs_tft(
        LogType type,
        uint32_t page,
        uint32_t entries_per_page,
        char *buffer,
        size_t buffer_size);

    static uint32_t get_log_entry_count(
        LogType type);


    // ========================================================
    // LOG MANAGEMENT
    // ========================================================

    static bool delete_logs(
        LogType type);

    static bool clear_old_logs(
        LogType type);

    static LogStorageStats get_log_stats(
        LogType type);


    // ========================================================
    // HELPERS
    // ========================================================

    static const char *get_log_path(
        LogType type);

    static uint64_t get_log_limit(
        LogType type);

    static const char *level_to_string(
        LogLevel level);


    // ========================================================
    // STATE
    // ========================================================

    static bool initialized;

    static uint64_t system_log_limit;
    static uint64_t activity_log_limit;
};