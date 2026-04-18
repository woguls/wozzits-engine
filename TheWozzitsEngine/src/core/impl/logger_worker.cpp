#include "../logger_worker.h"
#include <cstdio>

namespace WZ::core
{
    LoggerWorker::LoggerWorker()
    {
        callback = [](LogLevel level, const char *msg)
        {
            const char *level_str = "";

            switch (level)
            {
            case LogLevel::Debug:
                level_str = "DEBUG";
                break;
            case LogLevel::Info:
                level_str = "INFO";
                break;
            case LogLevel::Warning:
                level_str = "WARN";
                break;
            case LogLevel::Error:
                level_str = "ERROR";
                break;
            case LogLevel::Critical:
                level_str = "CRITICAL";
                break;
            }

            std::fprintf(stderr, "[%s] %s\n", level_str, msg);
        };
    }

    LoggerWorker::~LoggerWorker()
    {
        stop();
    }

    void LoggerWorker::start()
    {
        running.store(true, std::memory_order_release);
        worker = std::thread(&LoggerWorker::run, this);
    }

    void LoggerWorker::stop()
    {
        running.store(false, std::memory_order_release);

        if (worker.joinable())
            worker.join();

        // final drain
        LogEvent e;
        while (queue.try_pop(e))
        {
            callback(e.level, e.message);
        }
    }

    void LoggerWorker::set_callback(LogSinkType type)
    {
        switch (type)
        {
        case LogSinkType::Stderr:
            callback = [](LogLevel level, const char *msg)
            {
                std::fprintf(stderr, "[%d] %s\n", (int)level, msg);
            };
            break;

        case LogSinkType::Debugger:
            callback = [](LogLevel level, const char *msg)
            {
                // platform specific hook
                // OutputDebugStringA(msg);
            };
            break;

        case LogSinkType::File:
            callback = [this](LogLevel level, const char *msg)
            {
                // if (file)
                //     std::fprintf(file, "%s\n", msg);
            };
            break;

        case LogSinkType::Null:
            callback = [](LogLevel, const char *) {};
            break;
        }
    }

    void LoggerWorker::push(LogEvent event)
    {
        queue.push(std::move(event));
    }

    void LoggerWorker::run()
    {
        LogEvent event;

        while (running.load(std::memory_order_acquire))
        {
            if (queue.try_pop(event))
            {
                callback(event.level, event.message);
            }
            else
            {
                std::this_thread::yield(); // simple backoff for now
            }
        }

        // drain remaining logs
        while (queue.try_pop(event))
        {
            callback(event.level, event.message);
        }
    }
}