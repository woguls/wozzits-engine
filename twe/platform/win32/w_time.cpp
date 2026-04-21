#include <windows.h>
#include <cstdint>

namespace wz::win32::time
{
    uint64_t ticks_per_second()
    {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return static_cast<uint64_t>(f.QuadPart);
    }

    uint64_t now_ticks()
    {
        LARGE_INTEGER c;
        QueryPerformanceCounter(&c);
        return static_cast<uint64_t>(c.QuadPart);
    }
}