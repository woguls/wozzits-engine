#include <utility>
#include <core/utilities/wozassert.h>

namespace wz::result
{
    enum class eResultCode
    {
        OK,

        Unknown,
        InvalidArgument,

        OutOfMemory,
        AllocRefused,

        PlatformError,

        NotInitialized,
        AlreadyInitialized,

        IOError,
        Timeout,

        InternalError
    };

    struct sResult
    {
        eResultCode code = eResultCode::OK;
        int platform_code = 0;
        const char *message = nullptr;

        bool ok() const
        {
            return code == eResultCode::OK;
        }
    };

    template <typename T>
    class Result
    {
    public:
        static Result success(T value)
        {
            Result r;
            r.m_ok = true;
            r.m_value = std::move(value);
            return r;
        }

        static Result failure(eResultCode code, const char *msg = nullptr)
        {
            Result r;
            r.m_ok = false;
            r.m_error = {code, 0, msg};
            return r;
        }

        bool ok() const { return m_ok; }

        const T &value() const
        {
            WZ_PLATFORM_ASSERT(m_ok, "Accessing value on failed Result");
            return m_value;
        }

        const sResult &error() const
        {
            WZ_PLATFORM_ASSERT(!m_ok, "Accessing error on successful Result");
            return m_error;
        }

    private:
        bool m_ok = false;

        // NOTE: both exist, but only ONE is valid depending on m_ok
        T m_value{};
        sResult m_error{};
    };

    template <>
    class Result<void>
    {
    public:
        static Result success()
        {
            Result r;
            r.m_ok = true;
            return r;
        }

        static Result failure(eResultCode code, const char *msg = nullptr)
        {
            Result r;
            r.m_ok = false;
            r.m_error = {code, 0, msg};
            return r;
        }

        bool ok() const { return m_ok; }

        const sResult &error() const
        {
            WZ_PLATFORM_ASSERT(!m_ok, "Accessing error on success");
            return m_error;
        }

    private:
        bool m_ok = false;
        sResult m_error{};
    };
}