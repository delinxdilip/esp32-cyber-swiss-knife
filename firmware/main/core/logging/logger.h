#pragma once

#include <stdarg.h>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

enum class LogModule
{
    SYSTEM,
    WIFI,
    BLUETOOTH,
    NETWORK,
    SECURITY,
    STORAGE,
    WEB,
    HARDWARE
};

class Logger
{
public:
    static void init();

    static void set_level(LogLevel level);
    static LogLevel get_level();

    static void log(
        LogLevel level,
        LogModule module,
        const char *format,
        ...);

private:
    static LogLevel minimum_level;

    static const char *level_to_string(LogLevel level);
    static char level_to_char(LogLevel level);
    static const char *module_to_string(LogModule module);
};


// Logging macros
#define LOG_DEBUG(module, ...) \
    Logger::log(LogLevel::DEBUG, LogModule::module, __VA_ARGS__)

#define LOG_INFO(module, ...) \
    Logger::log(LogLevel::INFO, LogModule::module, __VA_ARGS__)

#define LOG_WARN(module, ...) \
    Logger::log(LogLevel::WARN, LogModule::module, __VA_ARGS__)

#define LOG_ERROR(module, ...) \
    Logger::log(LogLevel::ERROR, LogModule::module, __VA_ARGS__)
    