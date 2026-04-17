#include "../logging.h"

#include <iostream>
#include <cstdarg>
#include <cstring>
#include <cassert>
#include <thread>
#include <sstream>
#include <vector>

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

    // Thread-local log buffer structure
    struct ThreadLocalLogBuffer
    {
        std::vector<std::pair<LogLevel, std::string>> messages;

        // Maximum messages to store before auto-flush
        static constexpr size_t MAX_MESSAGES = 1000;

        void add_message(LogLevel level, const std::string &message)
        {
            messages.emplace_back(level, message);
            if (messages.size() >= MAX_MESSAGES)
            {
                flush();
            }
        }

        void flush()
        {
            if (messages.empty())
                return;

            // Get the current callback
            static std::mutex callback_mutex;
            std::lock_guard<std::mutex> lock(callback_mutex);
            auto callback = g_log_callback.load(std::memory_order_acquire);

            // Process all messages
            for (const auto &msg : messages)
            {
                callback(msg.first, msg.second.c_str());
            }

            messages.clear();
        }
    };

    // Thread-local storage for log buffers
    thread_local ThreadLocalLogBuffer g_thread_local_buffer;

    // Global callback with atomic access
    std::atomic<LogCallback> g_log_callback{default_log_callback};

} // anonymous namespace

// Public API
void set_log_callback(LogCallback callback)
{
    g_log_callback.store(callback ? callback : default_log_callback, std::memory_order_release);
}

void log_debug(const char *message)
{
    g_thread_local_buffer.add_message(LogLevel::Debug, message ? message : "");
}

void log_info(const char *message)
{
    g_thread_local_buffer.add_message(LogLevel::Info, message ? message : "");
}

void log_warning(const char *message)
{
    g_thread_local_buffer.add_message(LogLevel::Warning, message ? message : "");
}

void log_error(const char *message)
{
    g_thread_local_buffer.add_message(LogLevel::Error, message ? message : "");
}

void log_critical(const char *message)
{
    g_thread_local_buffer.add_message(LogLevel::Critical, message ? message : "");
}

void flush_thread_local_logs()
{
    g_thread_local_buffer.flush();
}
