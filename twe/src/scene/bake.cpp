#include <wozzits/scene/bake.h>
#include <algo/next.h>
#include <wozzits/math/mat4.h>
#include <render/render.h>
#include <wozzits/scene/scene.h>
#include <iostream>

namespace wz::scene::bake
{
    void bake_transforms(
        const wz::core::containers::Buffer<wz::scene::TransformNode>& nodes,
        wz::core::containers::Buffer<wz::core::render::ObjectData>& out)
    {
        using namespace wz::core::render;

        const auto* data = nodes.data();
        const uint32_t count = nodes.count();


        for (uint32_t i = 0; i < count; ++i)
        {
            const auto& node = data[i];

            // skip non-root chains if parent exists (simple first pass rule)
            // optional: you may later replace with traversal order
            if (node.parent != INVALID_TRANSFORM_NODE)
                continue;

            // emit object data
            out.push(ObjectData{
                node.world,
                i,
                EffectBits{},
                RenderFlags{}
                });
        }
    }

    using namespace wz::core::containers;
    using namespace wz::scene;

    void bake_transforms_v2(
        const Buffer<TransformNode>& nodes,
        const Buffer<Object>& objects,
        Buffer<wz::core::render::ObjectData>& out)
    {
        using namespace wz::core::render;

        auto* node_data = nodes.data();

        wz::core::algo::next::transform(
            objects,
            out,
            [&](const Object& obj)
            {
                const TransformNode& node = node_data[obj.node];

                ObjectData r{};
                r.world = node.world;
                r.scene_node = obj.node;
                r.effect_mask = (EffectBits)0;
                r.render_flags = (RenderFlags)0;

                return r;
            }
        );
    }
}