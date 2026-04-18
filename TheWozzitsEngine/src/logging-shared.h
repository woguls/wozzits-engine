#pragma once

namespace WZ
{
    enum class LogLevel
    {
        Debug = 0,
        Info,
        Warning,
        Error,
        Critical
    };

    enum class LogSinkType
    {
        Stderr = 0,
        File,
        Debugger,
        Null
    };

    struct LogEvent
    {
        LogLevel level;
        const char *message;
    };
}