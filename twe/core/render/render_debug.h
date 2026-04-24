#pragma once

namespace wz::core::render::debug
{
    struct DebugStats
    {
        uint32_t objects = 0;
        uint32_t draws = 0;
        uint32_t passes = 0;
    };

    static DebugStats g_stats;

    void submit(const RenderIR& ir)
    {
        g_stats.objects = ir.objects.count;
        g_stats.draws = ir.draws.count;
        g_stats.passes = ir.passes.count;
    }

    const DebugStats& debug_stats()
    {
        return wz::core::render::debug::g_stats;
    }
}