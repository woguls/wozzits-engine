// Helper macros for common patterns
#define WZ_LOG_RETURN_IF_NULL(ptr, msg) \
    if (!(ptr))                  \
    return WZ::Result<decltype(ptr)>::failure(WZ::ErrorCode::InvalidArgument, msg ? msg : "Null pointer")

#define WZ_LOG_RETURN_IF_FALSE(cond, msg) \
    if (!(cond))                   \
    return WZ::Result<void>::failure(WZ::ErrorCode::InvalidArgument, msg ? msg : "Condition failed")

#define WZ_PLATFORM_ASSERT(cond, msg)                                                           \
    do                                                                                       \
    {                                                                                        \
        if (!(cond))                                                                         \
        {                                                                                    \
            WZ::log_error(std::string("Assertion failed: ") + #cond + " - " + (msg ? msg : "")); \
            assert(cond);                                                                    \
        }                                                                                    \
    } while (0)


