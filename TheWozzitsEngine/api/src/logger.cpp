#include "Logger.h"
#include "../core/logging.h"

namespace WZ
{
    Logger::Logger()
        : impl(std::make_unique<core::LoggerWorker>())
    {
        impl->start();
    }

    Logger::~Logger()
    {
    }

    void Logger::log(LogLevel level, std::string_view message)
    {
        impl->push(
            WZ::LogEvent{
                static_cast<WZ::LogLevel>(level),
                message.data()});
    }

    void Logger::set_callback(LogSinkType type)
    {
        impl->set_callback(type);
    }

    void Logger::debug(std::string_view msg) { log(LogLevel::Debug, msg); }
    void Logger::info(std::string_view msg) { log(LogLevel::Info, msg); }
    void Logger::warn(std::string_view msg) { log(LogLevel::Warning, msg); }
    void Logger::error(std::string_view msg) { log(LogLevel::Error, msg); }
    void Logger::critical(std::string_view msg) { log(LogLevel::Critical, msg); }
}