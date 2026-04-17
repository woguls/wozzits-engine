#include "logging.h"

#include <iostream>
#include <cstdarg>
#include <cstring>
#include <cassert>

namespace
{

    // Default log callback: prints to stderr
    void default_log_callback(LogLevel level, const char *message)
    {
        const char *level_str = nullptr;
        switch (level)
        {
        case LogLevel::Debug:
            level_str = "DEBUG";
            break;
        case LogLevel::Info:
            level_str = "INFO";
            break;
        case LogLevel::Warning:
            level_str = "WARNING";
            break;
        case LogLevel::Error:
            level_str = "ERROR";
            break;
        case LogLevel::Critical:
            level_str = "CRITICAL";
            break;
        }
        std::fprintf(stderr, "[%s] %s\n", level_str, message);
    }

    LogCallback g_log_callback = default_log_callback;

} // anonymous namespace

// Public API
void set_log_callback(LogCallback callback)
{
    g_log_callback = callback ? callback : default_log_callback;
}

void log_debug(const char *message)
{
    g_log_callback(LogLevel::Debug, message);
}

void log_info(const char *message)
{
    g_log_callback(LogLevel::Info, message);
}

void log_warning(const char *message)
{
    g_log_callback(LogLevel::Warning, message);
}

void log_error(const char *message)
{
    g_log_callback(LogLevel::Error, message);
}

void log_critical(const char *message)
{
    g_log_callback(LogLevel::Critical, message);
}
