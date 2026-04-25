#include <render/frame.h>
#include <math/w_math.h>
#include <algorithm>

namespace
{
    inline bool inside_frustum(
        const wz::math::Frustum& f,
        const wz::math::Vec3& center,
        float radius
    )
    {
        for (int i = 0; i < 6; i++)
        {
            const auto& p = f.planes[i];

            float dist =
                p.normal.x * center.x +
                p.normal.y * center.y +
                p.normal.z * center.z +
                p.distance;

            // sphere is fully outside plane
            if (dist < -radius)
                return false;
        }

        return true;
    }
}


namespace wz::render
{
    void FrameBuilder::cull(
        const wz::core::render::RenderIR& ir,
        RenderFrame& out_frame
    )
    {
        const auto& view = ir.views.data[0];
        const auto& primitives = ir.primitives;
        const auto& bounds = ir.bounds;
        const auto& objects = ir.objects;

        auto& out = out_frame.visible_primitives;
        out.reset();

        for (uint32_t i = 0; i < primitives.count; i++)
        {
            const auto& prim = primitives.data[i];
            const auto& b = bounds.data[prim.bounds_id];
            const auto& obj = objects.data[prim.object_id];

            // ------------------------------------------------------------
            // WORLD SPACE POSITION
            // ------------------------------------------------------------
            wz::math::Vec3 world_center = {
                b.center.x + obj.transform.position.x,
                b.center.y + obj.transform.position.y,
                b.center.z + obj.transform.position.z
            };

            // ------------------------------------------------------------
            // VIEW SPACE TRANSFORM (camera space)
            // ------------------------------------------------------------
            wz::math::Vec3 v = to_view_space(view.view, world_center);

            // ------------------------------------------------------------
            // FRUSTUM TEST (now correctly in view space)
            // ------------------------------------------------------------
            bool rejected = false;
            printf("prim %u rejected=%d\n", i, rejected);
            for (int j = 0; j < 6; j++)
            {
                const auto& p = view.frustum.planes[j];

                float dist =
                    p.normal.x * v.x +
                    p.normal.y * v.y +
                    p.normal.z * v.z +
                    p.distance;

                if (dist < -b.radius)
                {
                    rejected = true;
                    break;
                }
            }

            if (!rejected)
            {
                out.push(i);
            }
        }
    }

    void FrameBuilder::classify(
        const wz::core::render::RenderIR& ir,
        RenderFrame& out_frame
    )
    {
        const auto& primitives = ir.primitives;
        const auto& objects = ir.objects;
        const auto& materials = ir.materials;
        const auto& bounds = ir.bounds;

        const auto& visible = out_frame.visible_primitives;
        auto& draws = out_frame.draws;

        draws.reset();

        for (uint32_t i = 0; i < visible.count; i++)
        {
            uint32_t prim_id = visible.data[i];
            const auto& prim = primitives.data[prim_id];

            const auto& obj = objects.data[prim.object_id];
            const auto& mat = materials.data[prim.material_id];
            const auto& bnd = bounds.data[prim.bounds_id];

            uint32_t pipe = mat.pipeline_state_id;

            float depth_f =
                bnd.center.x * bnd.center.x +
                bnd.center.y * bnd.center.y +
                bnd.center.z * bnd.center.z;
            uint32_t depth = (uint32_t)(depth_f * 1000.0f);

            uint64_t key = wz::core::render::make_sort_key(
                pipe,
                prim.material_id,
                depth
            );

            uint32_t pass_mask = 1u << 0;

            draws.push({
                key,
                prim_id,
                pass_mask
                });
        }
    }

    void FrameBuilder::sort(RenderFrame& out_frame)
    {
        auto& draws = out_frame.draws;

        // TODO: if needed: std::stable_sort(... by pass_mask);
        // Later upgrade path: 
        //    radix sort(64 - bit)
        //    or bucketed sort per pass
        std::sort(
            draws.data,
            draws.data + draws.count,
            [](const wz::core::render::DrawRef& a,
                const wz::core::render::DrawRef& b)
            {
                return a.sort_key < b.sort_key;
            }
        );
    }


    // TODO:
    //      PassType(opaque / transparent / shadow)
    //      pipeline_state
    //      lighting mode
    //      render target
    //      compute vs graphics dispatch
    void FrameBuilder::bin(
        const wz::core::render::RenderIR& ir,
        RenderFrame& out_frame
    )
    {
        auto& draws = out_frame.draws;
        auto& passes = out_frame.passes;

        passes.reset();

        if (draws.count == 0)
            return;

        uint32_t range_start = 0;
        uint32_t current_pass = draws.data[0].pass_mask;

        for (uint32_t i = 0; i < draws.count; i++)
        {
            const auto& d = draws.data[i];

            if (d.pass_mask != current_pass)
            {
                passes.push({
                    wz::core::render::PassType::Opaque,
                    range_start,
                    i - range_start,
                    current_pass,
                    wz::core::render::SortMode::None,
                    wz::core::render::DispatchType::Direct,
                    wz::core::render::LightingMode::Forward,
                    {}
                });

                range_start = i;
                current_pass = d.pass_mask;
            }
        }

        passes.push({
            wz::core::render::PassType::Opaque,
            range_start,
            draws.count - range_start,
            current_pass,
            wz::core::render::SortMode::None,
            wz::core::render::DispatchType::Direct,
            wz::core::render::LightingMode::Forward,
            {}
        });
    }
}