#include <render/frame.h>
//#include <math/w_math.h>
#include <algorithm>
#include <wozzits/math/camera.h>
#include <wozzits/math/frustum.h>
#include <wozzits/math/quaternion.h>
#include <wozzits/math/vec3.h>

namespace
{
    wz::math::Vec3 transform_point(
        const wz::math::Transform& t,
        const wz::math::Vec3& p)
    {
        // scale (simple)
        wz::math::Vec3 s = {
            p.x * t.scale.x,
            p.y * t.scale.y,
            p.z * t.scale.z
        };

        // rotate (placeholder for now)
        wz::math::Vec3 r = wz::math::rotate(t.rotation, s);

        // translate
        return {
            r.x + t.position.x,
            r.y + t.position.y,
            r.z + t.position.z
        };
    }

    inline bool is_in_front(const wz::math::Vec3& v)
    {
        return v.z < 0.0f;
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

        // ------------------------------------------------------------
        // BUILD FRUSTUM (ONLY SOURCE OF TRUTH)
        // ------------------------------------------------------------
        wz::math::Mat4 vp =
            wz::math::mul(view.proj, view.view);

        wz::math::Frustum fr =
            wz::math::frustum_from_view_projection(vp);

        // ------------------------------------------------------------
        // CULL LOOP
        // ------------------------------------------------------------
        for (uint32_t i = 0; i < primitives.count; i++)
        {
            const auto& prim = primitives.data[i];
            const auto& b = bounds.data[prim.bounds_id];
            const auto& obj = objects.data[prim.object_id];

            // WORLD → VIEW SPACE
            wz::math::Vec3 world_center =
                wz::math::mul_point(obj.world, b.center);

            bool rejected = false;

            for (int j = 0; j < 6; j++)
            {
                const auto& p = fr.planes[j];

                float dist =
                    wz::math::dot(p.normal, world_center) + p.distance;

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
        const auto& view = ir.views.data[0];
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

            wz::math::Vec3 world_center =
                wz::math::mul_point(obj.world, bnd.center);

            wz::math::Vec3 view_center =
                wz::math::mul_point(view.view, world_center);

            float depth_f = -view_center.z;

            if (depth_f < 0.0f)
                depth_f = 0.0f;

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
        std::stable_sort(
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