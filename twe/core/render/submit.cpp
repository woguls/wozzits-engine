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

    void bake_scene(const wz::scene::Scene& scene, RenderIR& ir)
    {
        ir.objects.reset();

        for (uint32_t i = 0; i < scene.nodes.count; i++)
        {
            ObjectData obj{};
            obj.scene_node = i;
            obj.effect_mask = {};
            obj.render_flags = {};

            ir.objects.push(obj);
        }

        // primitives/materials would be filled similarly
    }
}