#include "log_manager.h"

#include <cstdio>
#include <cstring>

#include "esp_log.h"
#include "esp_timer.h"

#include "core/config/config_manager.h"
#include "core/storage/storage_manager.h"

namespace
{
    constexpr const char *TAG = "LogManager";

    constexpr const char *SYSTEM_LOG_FILE =
        "/spiffs/system.log";

    constexpr const char *ACTIVITY_LOG_FILE =
        "/spiffs/activity.log";

    constexpr const char *SYSTEM_TEMP_FILE =
        "/spiffs/system.tmp";

    constexpr const char *ACTIVITY_TEMP_FILE =
        "/spiffs/activity.tmp";

    // Percentage of filesystem reserved for logs.
    constexpr uint8_t LOG_SPACE_PERCENT = 50;

    // Split log space equally between system/activity logs.
    constexpr uint8_t LOG_SPLIT_PERCENT = 50;

    // Minimum size assigned to a log.
    constexpr uint64_t MIN_LOG_SIZE = 4096;

    /*
     * Fixed-size buffer used when reading files.
     *
     * IMPORTANT:
     * Log size does not increase this buffer.
     */
    constexpr size_t LOG_BUFFER_SIZE = 1024;

    /*
     * Maximum individual log line size.
     *
     * source[32] + message[128] + timestamp/level/separators
     * comfortably fit inside this.
     */
    constexpr size_t LOG_LINE_SIZE = 256;
}


// ============================================================
// STATIC MEMBERS
// ============================================================

bool LogManager::initialized = false;

uint64_t LogManager::system_log_limit = 0;
uint64_t LogManager::activity_log_limit = 0;


// ============================================================
// INITIALIZE
// ============================================================

bool LogManager::initialize()
{
    if (initialized)
    {
        return true;
    }

    if (!StorageManager::is_initialized())
    {
        ESP_LOGE(
            TAG,
            "StorageManager is not initialized");

        return false;
    }

    const uint64_t total_space =
        StorageManager::get_total_space();

    if (total_space == 0)
    {
        ESP_LOGE(
            TAG,
            "Filesystem has no available space");

        return false;
    }

    /*
     * Calculate log storage dynamically.
     *
     * Example:
     *
     * Filesystem = 5 MB
     * Logs       = 50%
     * System     = 25%
     * Activity   = 25%
     */
    const uint64_t log_space =
        (total_space * LOG_SPACE_PERCENT) / 100;

    const uint64_t per_log_space =
        (log_space * LOG_SPLIT_PERCENT) / 100;

    system_log_limit =
        (per_log_space < MIN_LOG_SIZE)
            ? MIN_LOG_SIZE
            : per_log_space;

    activity_log_limit =
        (per_log_space < MIN_LOG_SIZE)
            ? MIN_LOG_SIZE
            : per_log_space;

    initialized = true;

    ESP_LOGI(
        TAG,
        "LogManager initialized");

    ESP_LOGI(
        TAG,
        "System log limit: %llu bytes",
        static_cast<unsigned long long>(
            system_log_limit));

    ESP_LOGI(
        TAG,
        "Activity log limit: %llu bytes",
        static_cast<unsigned long long>(
            activity_log_limit));

    return true;
}


// ============================================================
// LEVEL TO STRING
// ============================================================

const char *LogManager::level_to_string(
    LogLevel level)
{
    switch (level)
    {
        case LogLevel::INFO:
            return "INFO";

        case LogLevel::WARN:
            return "WARN";

        case LogLevel::ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}


// ============================================================
// GET LOG PATH
// ============================================================

const char *LogManager::get_log_path(
    LogType type)
{
    switch (type)
    {
        case LogType::SYSTEM:
            return SYSTEM_LOG_FILE;

        case LogType::ACTIVITY:
            return ACTIVITY_LOG_FILE;

        default:
            return nullptr;
    }
}


// ============================================================
// GET LOG LIMIT
// ============================================================

uint64_t LogManager::get_log_limit(
    LogType type)
{
    switch (type)
    {
        case LogType::SYSTEM:
            return system_log_limit;

        case LogType::ACTIVITY:
            return activity_log_limit;

        default:
            return 0;
    }
}


// ============================================================
// ADD SYSTEM LOG
// ============================================================

bool LogManager::add_system_log(
    LogLevel level,
    const char *source,
    const char *message)
{
    return add_log(
        LogType::SYSTEM,
        level,
        source,
        message);
}


// ============================================================
// ADD ACTIVITY LOG
// ============================================================

bool LogManager::add_activity_log(
    LogLevel level,
    const char *source,
    const char *message)
{
    return add_log(
        LogType::ACTIVITY,
        level,
        source,
        message);
}


// ============================================================
// ADD LOG
// ============================================================

bool LogManager::add_log(
    LogType type,
    LogLevel level,
    const char *source,
    const char *message)
{
    if (!initialized)
    {
        return false;
    }

    if (source == nullptr ||
        message == nullptr)
    {
        return false;
    }

    const char *path =
        get_log_path(type);

    if (path == nullptr)
    {
        return false;
    }

    FILE *file =
        fopen(path, "a");

    if (file == nullptr)
    {
        ESP_LOGE(
            TAG,
            "Failed to open log file: %s",
            path);

        return false;
    }

    const int written =
        fprintf(
            file,
            "%llu|%s|%s|%s\n",
            static_cast<unsigned long long>(
                esp_timer_get_time() / 1000000ULL),
            level_to_string(level),
            source,
            message);

    fclose(file);

    if (written < 0)
    {
        ESP_LOGE(
            TAG,
            "Failed to write log entry");

        return false;
    }

    check_storage();

    return true;
}


// ============================================================
// OPEN LOGS
// ============================================================

bool LogManager::open_logs(
    LogType type,
    LogReadHandle &handle)
{
    if (!initialized)
    {
        return false;
    }

    if (handle.active)
    {
        return false;
    }

    const char *path =
        get_log_path(type);

    if (path == nullptr)
    {
        return false;
    }

    FILE *file =
        fopen(path, "r");

    if (file == nullptr)
    {
        return false;
    }

    handle.file = file;
    handle.type = type;
    handle.active = true;

    return true;
}


// ============================================================
// OPEN SYSTEM LOGS
// ============================================================

bool LogManager::open_system_logs(
    LogReadHandle &handle)
{
    return open_logs(
        LogType::SYSTEM,
        handle);
}


// ============================================================
// OPEN ACTIVITY LOGS
// ============================================================

bool LogManager::open_activity_logs(
    LogReadHandle &handle)
{
    return open_logs(
        LogType::ACTIVITY,
        handle);
}


// ============================================================
// READ LOG CHUNK
// ============================================================

size_t LogManager::read_log_chunk(
    LogReadHandle &handle,
    char *buffer,
    size_t buffer_size)
{
    if (!initialized)
    {
        return 0;
    }

    if (!handle.active ||
        handle.file == nullptr)
    {
        return 0;
    }

    if (buffer == nullptr ||
        buffer_size == 0)
    {
        return 0;
    }

    return fread(
        buffer,
        1,
        buffer_size,
        handle.file);
}


// ============================================================
// CLOSE LOGS
// ============================================================

void LogManager::close_logs(
    LogReadHandle &handle)
{
    if (!handle.active ||
        handle.file == nullptr)
    {
        return;
    }

    fclose(handle.file);

    handle.file = nullptr;
    handle.active = false;
}


// ============================================================
// GET LOG ENTRY COUNT
// ============================================================

uint32_t LogManager::get_log_entry_count(
    LogType type)
{
    if (!initialized)
    {
        return 0;
    }

    const char *path =
        get_log_path(type);

    if (path == nullptr)
    {
        return 0;
    }

    FILE *file =
        fopen(path, "r");

    if (file == nullptr)
    {
        return 0;
    }

    uint32_t count = 0;

    char buffer[LOG_BUFFER_SIZE];

    while (true)
    {
        const size_t bytes_read =
            fread(
                buffer,
                1,
                sizeof(buffer),
                file);

        if (bytes_read == 0)
        {
            break;
        }

        for (size_t i = 0;
             i < bytes_read;
             ++i)
        {
            if (buffer[i] == '\n')
            {
                ++count;
            }
        }

        if (bytes_read < sizeof(buffer))
        {
            break;
        }
    }

    fclose(file);

    return count;
}


// ============================================================
// READ TFT LOG PAGE
// ============================================================

bool LogManager::read_logs_tft(
    LogType type,
    uint32_t page,
    uint32_t entries_per_page,
    char *buffer,
    size_t buffer_size)
{
    if (!initialized)
    {
        return false;
    }

    if (buffer == nullptr ||
        buffer_size == 0)
    {
        return false;
    }

    if (entries_per_page == 0)
    {
        return false;
    }

    buffer[0] = '\0';

    const char *path =
        get_log_path(type);

    if (path == nullptr)
    {
        return false;
    }

    /*
     * First determine how many entries exist.
     *
     * This only uses a fixed 1 KB buffer.
     */
    const uint32_t total_entries =
        get_log_entry_count(type);

    if (total_entries == 0)
    {
        return true;
    }

    /*
     * Pages are ordered newest → oldest.
     *
     * Example:
     *
     * Page 0 = newest entries
     * Page 1 = previous entries
     * Page 2 = older entries
     */
    const uint64_t page_start_from_newest =
        static_cast<uint64_t>(page) *
        entries_per_page;

    if (page_start_from_newest >= total_entries)
    {
        return false;
    }

    /*
     * Determine the first entry to display
     * using the file's oldest → newest order.
     */
    uint32_t start_entry = 0;

    if (total_entries >
        page_start_from_newest +
        entries_per_page)
    {
        start_entry =
            total_entries -
            static_cast<uint32_t>(
                page_start_from_newest +
                entries_per_page);
    }
    else
    {
        start_entry = 0;
    }

    uint32_t end_entry =
        total_entries -
        static_cast<uint32_t>(
            page_start_from_newest);

    /*
     * Open the file again for the actual page read.
     */
    FILE *file =
        fopen(path, "r");

    if (file == nullptr)
    {
        return false;
    }

    char line[LOG_LINE_SIZE];

    uint32_t current_entry = 0;
    size_t output_length = 0;

    while (current_entry < end_entry)
    {
        if (fgets(
            line,
            sizeof(line),
            file) == nullptr)
        {
            break;
        }

        if (current_entry >= start_entry)
        {
            const size_t line_length =
                strlen(line);

            /*
             * Never allow the TFT buffer to overflow.
             */
            const size_t remaining =
                buffer_size -
                output_length -
                1;

            if (remaining == 0)
            {
                break;
            }

            const size_t copy_length =
                (line_length < remaining)
                    ? line_length
                    : remaining;

            memcpy(
                buffer + output_length,
                line,
                copy_length);

            output_length += copy_length;

            buffer[output_length] = '\0';

            /*
             * If the line was too large for the remaining
             * buffer, stop here rather than corrupting memory.
             */
            if (copy_length < line_length)
            {
                break;
            }
        }

        ++current_entry;
    }

    fclose(file);

    return true;
}


// ============================================================
// VIEW SYSTEM LOGS ON TFT
// ============================================================

bool LogManager::view_system_logs_tft(
    uint32_t page,
    uint32_t entries_per_page,
    char *buffer,
    size_t buffer_size)
{
    return read_logs_tft(
        LogType::SYSTEM,
        page,
        entries_per_page,
        buffer,
        buffer_size);
}


// ============================================================
// VIEW ACTIVITY LOGS ON TFT
// ============================================================

bool LogManager::view_activity_logs_tft(
    uint32_t page,
    uint32_t entries_per_page,
    char *buffer,
    size_t buffer_size)
{
    return read_logs_tft(
        LogType::ACTIVITY,
        page,
        entries_per_page,
        buffer,
        buffer_size);
}


// ============================================================
// GET LOG STATS
// ============================================================

LogStorageStats LogManager::get_log_stats(
    LogType type)
{
    LogStorageStats stats = {};

    if (!initialized)
    {
        return stats;
    }

    const char *path =
        get_log_path(type);

    if (path == nullptr)
    {
        return stats;
    }

    const uint64_t limit =
        get_log_limit(type);

    stats.total = limit;

    FILE *file =
        fopen(path, "rb");

    if (file == nullptr)
    {
        stats.used = 0;
        stats.remaining = limit;
        stats.entry_count = 0;
        stats.full = false;

        return stats;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return stats;
    }

    const long file_size =
        ftell(file);

    if (file_size < 0)
    {
        fclose(file);
        return stats;
    }

    stats.used =
        static_cast<uint64_t>(file_size);

    fclose(file);

    if (stats.used >= limit)
    {
        stats.used = limit;
        stats.remaining = 0;
        stats.full = true;
    }
    else
    {
        stats.remaining =
            limit - stats.used;

        stats.full = false;
    }

    /*
     * Count entries using a fixed-size buffer.
     */
    stats.entry_count =
        get_log_entry_count(type);

    return stats;
}


// ============================================================
// SYSTEM LOG STATS
// ============================================================

LogStorageStats LogManager::get_system_log_stats()
{
    return get_log_stats(
        LogType::SYSTEM);
}


// ============================================================
// ACTIVITY LOG STATS
// ============================================================

LogStorageStats LogManager::get_activity_log_stats()
{
    return get_log_stats(
        LogType::ACTIVITY);
}


// ============================================================
// DELETE LOGS
// ============================================================

bool LogManager::delete_logs(
    LogType type)
{
    if (!initialized)
    {
        return false;
    }

    const char *path =
        get_log_path(type);

    if (path == nullptr)
    {
        return false;
    }

    if (remove(path) != 0)
    {
        /*
         * If the file doesn't exist,
         * consider the operation successful.
         */
        FILE *file =
            fopen(path, "r");

        if (file == nullptr)
        {
            return true;
        }

        fclose(file);

        ESP_LOGE(
            TAG,
            "Failed to delete log file");

        return false;
    }

    ESP_LOGI(
        TAG,
        "Deleted log file: %s",
        path);

    return true;
}


// ============================================================
// DELETE SYSTEM LOGS
// ============================================================

bool LogManager::delete_system_logs()
{
    return delete_logs(
        LogType::SYSTEM);
}


// ============================================================
// DELETE ACTIVITY LOGS
// ============================================================

bool LogManager::delete_activity_logs()
{
    return delete_logs(
        LogType::ACTIVITY);
}


// ============================================================
// CLEAR OLD LOGS
// ============================================================

bool LogManager::clear_old_logs(
    LogType type)
{
    if (!initialized)
    {
        return false;
    }

    const char *path =
        get_log_path(type);

    if (path == nullptr)
    {
        return false;
    }

    const char *temp_path =
        (type == LogType::SYSTEM)
            ? SYSTEM_TEMP_FILE
            : ACTIVITY_TEMP_FILE;

    FILE *source =
        fopen(path, "r");

    if (source == nullptr)
    {
        return true;
    }

    /*
     * First pass:
     * count entries.
     */
    uint64_t entry_count = 0;

    char buffer[LOG_BUFFER_SIZE];

    while (true)
    {
        const size_t bytes_read =
            fread(
                buffer,
                1,
                sizeof(buffer),
                source);

        if (bytes_read == 0)
        {
            break;
        }

        for (size_t i = 0;
             i < bytes_read;
             ++i)
        {
            if (buffer[i] == '\n')
            {
                ++entry_count;
            }
        }

        if (bytes_read < sizeof(buffer))
        {
            break;
        }
    }

    fclose(source);

    if (entry_count <= 1)
    {
        return true;
    }

    /*
     * Remove approximately the oldest 25%.
     */
    const uint64_t entries_to_remove =
        entry_count / 4;

    source =
        fopen(path, "r");

    if (source == nullptr)
    {
        return false;
    }

    FILE *destination =
        fopen(temp_path, "w");

    if (destination == nullptr)
    {
        fclose(source);

        return false;
    }

    char line[LOG_LINE_SIZE];

    uint64_t current_entry = 0;

    while (fgets(
        line,
        sizeof(line),
        source) != nullptr)
    {
        ++current_entry;

        if (current_entry > entries_to_remove)
        {
            fputs(
                line,
                destination);
        }
    }

    fclose(source);
    fclose(destination);

    if (remove(path) != 0)
    {
        remove(temp_path);

        ESP_LOGE(
            TAG,
            "Failed to remove old log file");

        return false;
    }

    if (rename(
        temp_path,
        path) != 0)
    {
        remove(temp_path);

        ESP_LOGE(
            TAG,
            "Failed to rename temporary log file");

        return false;
    }

    ESP_LOGI(
        TAG,
        "Old log entries cleared");

    return true;
}


// ============================================================
// CLEAR OLD SYSTEM LOGS
// ============================================================

bool LogManager::clear_old_system_logs()
{
    return clear_old_logs(
        LogType::SYSTEM);
}


// ============================================================
// CLEAR OLD ACTIVITY LOGS
// ============================================================

bool LogManager::clear_old_activity_logs()
{
    return clear_old_logs(
        LogType::ACTIVITY);
}


// ============================================================
// CHECK STORAGE
// ============================================================

bool LogManager::check_storage()
{
    if (!initialized)
    {
        return false;
    }

    const LogStorageStats system_stats =
        get_system_log_stats();

    const LogStorageStats activity_stats =
        get_activity_log_stats();

    const LoggingConfig logging_config =
        ConfigManager::get_logging_config();

    if (system_stats.full)
    {
        if (logging_config.system_auto_clear)
        {
            clear_old_system_logs();
        }
        else
        {
            ESP_LOGW(
                TAG,
                "System log storage is full");
        }
    }

    if (activity_stats.full)
    {
        if (logging_config.activity_auto_clear)
        {
            clear_old_activity_logs();
        }
        else
        {
            ESP_LOGW(
                TAG,
                "Activity log storage is full");
        }
    }

    return true;
}


// ============================================================
// HANDLE AUTO CLEAR
// ============================================================

bool LogManager::handle_auto_clear()
{
    if (!initialized)
    {
        return false;
    }

    const LoggingConfig logging_config =
        ConfigManager::get_logging_config();

    const LogStorageStats system_stats =
        get_system_log_stats();

    const LogStorageStats activity_stats =
        get_activity_log_stats();

    if (system_stats.full &&
        logging_config.system_auto_clear)
    {
        clear_old_system_logs();
    }

    if (activity_stats.full &&
        logging_config.activity_auto_clear)
    {
        clear_old_activity_logs();
    }

    return true;
}


// ============================================================
// INITIALIZATION STATUS
// ============================================================

bool LogManager::is_initialized()
{
    return initialized;
}