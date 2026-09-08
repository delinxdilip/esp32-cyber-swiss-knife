#include "logger.h"

#include <stdio.h>

#include "esp_timer.h"


LogLevel Logger::minimum_level =
    LogLevel::DEBUG;


// ============================================================
// INITIALIZE
// ============================================================

void Logger::init()
{
    minimum_level = LogLevel::DEBUG;

    printf("\n");
    printf("========================================\n");
    printf("          Logger initialized            \n");
    printf("========================================\n");
}


// ============================================================
// SET LEVEL
// ============================================================

void Logger::set_level(
    LogLevel level)
{
    minimum_level = level;
}


// ============================================================
// GET LEVEL
// ============================================================

LogLevel Logger::get_level()
{
    return minimum_level;
}


// ============================================================
// LOG
// ============================================================

void Logger::log(
    LogType type,
    LogLevel level,
    LogModule module,
    const char *format,
    ...)
{
    if (level < minimum_level)
    {
        return;
    }


    // ========================================================
    // TIMESTAMP
    // ========================================================

    int64_t timestamp_us =
        esp_timer_get_time();

    uint64_t timestamp_ms =
        static_cast<uint64_t>(
            timestamp_us / 1000);


    // ========================================================
    // CONSOLE OUTPUT
    // ========================================================

    printf(
        "[%10llu ms] [%c] [%s] ",
        static_cast<unsigned long long>(
            timestamp_ms),
        level_to_char(level),
        module_to_string(module));


    va_list args;

    va_start(args, format);

    vprintf(
        format,
        args);

    va_end(args);

    printf("\n");


    // ========================================================
    // PERSISTENT LOG
    // ========================================================

    /*
     * DEBUG logs are console-only.
     *
     * Persistent logs support:
     *
     * INFO
     * WARN
     * ERROR
     */

    if (level == LogLevel::DEBUG)
    {
        return;
    }


    // ========================================================
    // FORMAT MESSAGE
    // ========================================================

    char message[128];

    va_start(args, format);

    vsnprintf(
        message,
        sizeof(message),
        format,
        args);

    va_end(args);


    // ========================================================
    // SOURCE
    // ========================================================

    const char *source =
        module_to_string(module);


    // ========================================================
    // STORE LOG
    // ========================================================

    if (type == LogType::SYSTEM)
    {
        LogManager::add_system_log(
            level,
            source,
            message);
    }
    else
    {
        LogManager::add_activity_log(
            level,
            source,
            message);
    }
}


// ============================================================
// LEVEL TO STRING
// ============================================================

const char *Logger::level_to_string(
    LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "DEBUG";

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
// LEVEL TO CHARACTER
// ============================================================

char Logger::level_to_char(
    LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return 'D';

        case LogLevel::INFO:
            return 'I';

        case LogLevel::WARN:
            return 'W';

        case LogLevel::ERROR:
            return 'E';

        default:
            return '?';
    }
}


// ============================================================
// MODULE TO STRING
// ============================================================

const char *Logger::module_to_string(
    LogModule module)
{
    switch (module)
    {
        case LogModule::SYSTEM:
            return "SYSTEM";

        case LogModule::WIFI:
            return "WIFI";

        case LogModule::BLUETOOTH:
            return "BLUETOOTH";

        case LogModule::NETWORK:
            return "NETWORK";

        case LogModule::SECURITY:
            return "SECURITY";

        case LogModule::STORAGE:
            return "STORAGE";

        case LogModule::WEB:
            return "WEB";

        case LogModule::HARDWARE:
            return "HARDWARE";

        case LogModule::CONFIG:
            return "CONFIG";

        default:
            return "UNKNOWN";
    }
}