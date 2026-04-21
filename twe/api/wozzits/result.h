/**
 * @file result.h
 * @brief Lightweight result type for error handling without exceptions.
 *
 * Provides a simple `Result<T>` type used throughout the engine to represent
 * either a successful value or a structured error (`sResult`).
 */

/**
 * @todo Consider replacing with a zero-overhead expected-style implementation.
 *
 * Current design is intentionally simple and safe, but duplicates storage for
 * both success and error states. This is acceptable for early engine development,
 * but may become inefficient in hot paths (queues, async systems, job results).
 *
 * Future refactor may introduce:
 * - tagged union with manual lifetime control, or
 * - std::variant (if allowed), or
 * - custom expected<T, E>-style implementation.
 */

#pragma once

#include <utility>
#include <core/utilities/wozassert.h>

namespace wz::result
{
    /**
     * @brief Standardized result codes used across the engine.
     *
     * These represent high-level categories of failure or state outcomes.
     * They are independent of platform-specific error codes.
     */
    enum class eResultCode
    {
        OK, ///< Operation succeeded.

        Unknown,         ///< Unspecified or unexpected error.
        InvalidArgument, ///< Invalid input provided.

        OutOfMemory,  ///< System memory exhaustion.
        AllocRefused, ///< Allocation refused by policy or limit.

        PlatformError, ///< Underlying OS/platform failure.

        NotInitialized,     ///< System or module not initialized.
        AlreadyInitialized, ///< Duplicate initialization attempt.

        IOError, ///< File or device I/O failure.
        Timeout, ///< Operation timed out.

        InternalError ///< Unexpected internal engine failure.
    };

    /**
     * @brief Structured error information returned by failing operations.
     *
     * Contains a high-level error code, optional platform-specific code,
     * and an optional human-readable message.
     */
    struct sResult
    {
        eResultCode code = eResultCode::OK; ///< High-level result code.
        int platform_code = 0;              ///< Platform-specific error code (e.g. errno, GetLastError()).
        const char *message = nullptr;      ///< Optional diagnostic message.

        /**
         * @brief Returns whether the result represents success.
         */
        bool ok() const
        {
            return code == eResultCode::OK;
        }
    };

    /**
     * @brief Generic result type for operations that return a value or an error.
     *
     * @tparam T Type of the success value.
     *
     * A Result is either:
     * - a valid value of type T, or
     * - a structured error (`sResult`)
     */
    template <typename T>
    class Result
    {
    public:
        /**
         * @brief Create a successful result.
         * @param value Value to store.
         */
        static Result success(T value)
        {
            Result r;
            r.m_ok = true;
            r.m_value = std::move(value);
            return r;
        }

        /**
         * @brief Create a failed result.
         * @param code High-level error code.
         * @param msg Optional diagnostic message.
         */
        static Result failure(eResultCode code, const char *msg = nullptr)
        {
            Result r;
            r.m_ok = false;
            r.m_error = {code, 0, msg};
            return r;
        }

        /**
         * @brief Returns true if the operation succeeded.
         */
        bool ok() const { return m_ok; }

        /**
         * @brief Access the success value.
         *
         * @warning Only valid if ok() == true.
         */
        const T &value() const
        {
            WZ_PLATFORM_ASSERT(m_ok, "Accessing value on failed Result");
            return m_value;
        }

        /**
         * @brief Access error information.
         *
         * @warning Only valid if ok() == false.
         */
        const sResult &error() const
        {
            WZ_PLATFORM_ASSERT(!m_ok, "Accessing error on successful Result");
            return m_error;
        }

    private:
        bool m_ok = false; ///< True if result is successful.

        T m_value{};       ///< Stored value (valid only if m_ok == true).
        sResult m_error{}; ///< Stored error (valid only if m_ok == false).
    };

    /**
     * @brief Specialization of Result for void return types.
     *
     * Represents success or failure without returning a value.
     */
    template <>
    class Result<void>
    {
    public:
        /**
         * @brief Create a successful void result.
         */
        static Result success()
        {
            Result r;
            r.m_ok = true;
            return r;
        }

        /**
         * @brief Create a failed void result.
         * @param code High-level error code.
         * @param msg Optional diagnostic message.
         */
        static Result failure(eResultCode code, const char *msg = nullptr)
        {
            Result r;
            r.m_ok = false;
            r.m_error = {code, 0, msg};
            return r;
        }

        /**
         * @brief Returns true if operation succeeded.
         */
        bool ok() const { return m_ok; }

        /**
         * @brief Access error information.
         *
         * @warning Only valid if ok() == false.
         */
        const sResult &error() const
        {
            WZ_PLATFORM_ASSERT(!m_ok, "Accessing error on success");
            return m_error;
        }

    private:
        bool m_ok = false; ///< True if operation succeeded.
        sResult m_error{}; ///< Error details if failed.
    };
}