// api/time.h
#pragma once
#include <cstdint>
#ifdef WZ_ENABLE_TESTING
#include <chrono>
#endif
namespace wz::time
{
    using TimePoint = uint64_t;

    class TimeSource
    {
    public:
        static TimePoint now();
        static uint64_t ticks_per_second();

#ifdef WZ_ENABLE_TESTING
        static uint64_t tick_resolution_ns()
        {
            return std::chrono::steady_clock::period::num *
                   1'000'000'000ull /
                   std::chrono::steady_clock::period::den;
        }
#endif
    private:
        TimeSource() = delete;
    };
}

namespace wz::time
{
    struct TemporalFeature
    {
        // ─────────────────────────────
        // IDENTITY
        // ─────────────────────────────
        uint64_t frame_index;
        float total_time;
        float dt;

        // ─────────────────────────────
        // SMOOTHED TIME SIGNAL
        // ─────────────────────────────
        float dt_ema;
        float dt_variance;
        float jitter;

        // ─────────────────────────────
        // STABILITY / QUALITY
        // ─────────────────────────────
        float stability;
        float continuity;
        float time_quality;

        // ─────────────────────────────
        // FREQUENCY PROXIES
        // ─────────────────────────────
        float dt_derivative;
        float oscillation;

        // ─────────────────────────────
        // BURST DETECTION
        // ─────────────────────────────
        float spike_score;
        float burst_intensity;
    };
}