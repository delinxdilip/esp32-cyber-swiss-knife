#pragma once

#include <stdarg.h>
#include "log_manager.h"

enum class LogModule
{
    SYSTEM,
    WIFI,
    BLUETOOTH,
    NETWORK,
    SECURITY,
    STORAGE,
    WEB,
    HARDWARE,
    CONFIG
};

class Logger
{
public:
    static void init();

    static void set_level(LogLevel level);

    static LogLevel get_level();

    static void log(
        LogType type,
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


// ============================================================
// Logging Macros
// ============================================================

#define LOG_DEBUG(type, module, ...) Logger::log(LogType::type, LogLevel::DEBUG, LogModule::module, __VA_ARGS__)

#define LOG_INFO(type, module, ...) Logger::log(LogType::type, LogLevel::INFO, LogModule::module, __VA_ARGS__)

#define LOG_WARN(type, module, ...) Logger::log(LogType::type, LogLevel::WARN, LogModule::module, __VA_ARGS__)

#define LOG_ERROR(type, module, ...) Logger::log(LogType::type, LogLevel::ERROR, LogModule::module, __VA_ARGS__)