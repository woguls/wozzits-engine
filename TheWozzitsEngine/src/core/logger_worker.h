#pragma once

#include <atomic>
#include <thread>
#include <functional>

#include "mpsc_queue.h"
#include "../logging-shared.h"

namespace WZ::core
{

    class LoggerWorker
    {
    public:
        using Callback = std::function<void(LogLevel, const char *)>;

        LoggerWorker();
        ~LoggerWorker();

        LoggerWorker(const LoggerWorker &) = delete;
        LoggerWorker &operator=(const LoggerWorker &) = delete;

        void start();
        void stop();

        void push(LogEvent event);

        void set_callback(LogSinkType t);

    private:
        void run();

    private:
        MPSCQueue<LogEvent> queue;

        std::thread worker;
        std::atomic<bool> running{false};

        Callback callback;
    };
}