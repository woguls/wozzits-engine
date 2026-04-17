#pragma once

#include <cstdint>
#include <string>

// Log levels for the logging system
enum class LogLevel : uint8_t
{
    Debug = 0,
    Info,
    Warning,
    Error,
    Critical
};

// Function pointer type for custom logging callbacks
using LogCallback = void (*)(LogLevel level, const char *message);

// Set a custom logging callback; default logs to console
void set_log_callback(LogCallback callback);

// Core logging functions
void log_debug(const char *message);
void log_info(const char *message);
void log_warning(const char *message);
void log_error(const char *message);
void log_critical(const char *message);

// Error codes for platform operations
enum class ErrorCode : uint8_t
{
    Success = 0,
    InvalidArgument,
    OutOfMemory,
    PlatformFailure,
    FileNotFound,
    PermissionDenied,
    Timeout,
    Unknown
};

// Result type for operations that may fail
template <typename T>
struct Result
{
    bool success;
    ErrorCode code;
    std::string message;
    T value;

    // Constructors
    static Result<T> success(const T &val, const std::string &msg = "")
    {
        return {true, ErrorCode::Success, msg, val};
    }

    static Result<T> failure(ErrorCode code, const std::string &msg = "")
    {
        return {false, code, msg, T{}};
    }

    // Convenience accessors
    T &&get() && { return std::move(value); }
    const T &get() const & { return value; }
    operator bool() const { return success; }
};

// Specialization for void-returning operations
template <>
struct Result<void>
{
    bool success;
    ErrorCode code;
    std::string message;

    static Result<void> success(const std::string &msg = "")
    {
        return {true, ErrorCode::Success, msg};
    }

    static Result<void> failure(ErrorCode code, const std::string &msg = "")
    {
        return {false, code, msg};
    }

    operator bool() const { return success; }
};

// Helper macros for common patterns
#define RETURN_IF_NULL(ptr, msg) \
    if (!(ptr)) return Result<decltype(ptr)>::failure(ErrorCode::InvalidArgument, msg ? msg : "Null pointer")

#define RETURN_IF_FALSE(cond, msg) \
    if (!(cond)) return Result<void>::failure(ErrorCode::InvalidArgument, msg ? msg : "Condition failed")

#define PLATFORM_ASSERT(cond, msg)                                       \
    do                                                                   \
    {                                                                    \
        if (!(cond))                                                     \
        {                                                                \
            log_error("Assertion failed: " #cond " - "(msg ? msg : "")); \
            assert(cond);                                                \
        }                                                                \
    } while (0)