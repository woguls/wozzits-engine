#include <render/submit.h>
#include <render/render.h>
#include <render/render_debug.h>

namespace wz::core::render
{
    static debug::DebugStats g_stats;

    void submit(const wz::core::render::RenderIR& ir)
    {
        g_stats.objects = ir.objects.count;
        g_stats.draws = ir.draws.count;
        g_stats.passes = ir.passes.count;
    }
}