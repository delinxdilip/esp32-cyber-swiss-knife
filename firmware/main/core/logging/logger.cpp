#include "logger.h"

#include <stdio.h>

#include "esp_timer.h"

LogLevel Logger::minimum_level = LogLevel::DEBUG;

void Logger::init()
{
    minimum_level = LogLevel::DEBUG;

    printf("\n");
    printf("========================================\n");
    printf("          Logger initialized            \n");
    printf("========================================\n");
}

void Logger::set_level(LogLevel level)
{
    minimum_level = level;
}

LogLevel Logger::get_level()
{
    return minimum_level;
}

void Logger::log(
    LogLevel level,
    LogModule module,
    const char *format,
    ...)
{
    if (level < minimum_level)
    {
        return;
    }

    int64_t timestamp_us = esp_timer_get_time();
    uint64_t timestamp_ms =
        static_cast<uint64_t>(timestamp_us / 1000);

    printf(
        "[%10llu ms] [%c] [%s] ",
        static_cast<unsigned long long>(timestamp_ms),
        level_to_char(level),
        module_to_string(module));

    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);

    printf("\n");
}

const char *Logger::level_to_string(LogLevel level)
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

char Logger::level_to_char(LogLevel level)
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

const char *Logger::module_to_string(LogModule module)
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

        default:
            return "UNKNOWN";
    }
}