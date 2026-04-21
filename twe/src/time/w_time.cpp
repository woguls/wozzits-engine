#include <wozzits/w_time.h>
#include <win32/w_time.h> // or declare functions directly

namespace wz::time
{
    TimePoint TimeSource::now()
    {
        return wz::win32::time::now_ticks();
    }

    uint64_t TimeSource::ticks_per_second()
    {
        return wz::win32::time::ticks_per_second();
    }
}