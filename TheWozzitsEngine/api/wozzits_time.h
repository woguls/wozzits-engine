#pragma once

#include <cstdint>
#include <chrono>

namespace WZ::Time
{
    using TimePoint = uint64_t; // nanoseconds (monotonic)
    using DurationNs = uint64_t;

    class Clock
    {
    public:
        static TimePoint now()
        {
            using namespace std::chrono;

            return static_cast<TimePoint>(
                duration_cast<nanoseconds>(
                    steady_clock::now().time_since_epoch())
                    .count());
        }

#ifdef WZ_ENABLE_TESTING
        static uint64_t tick_resolution_ns()
        {
            return 1; // logical resolution assumption
        }
#endif
    };
}