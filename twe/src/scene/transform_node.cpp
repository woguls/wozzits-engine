#include <wozzits/scene/transform_node.h>

namespace wz::scene
{
    inline wz::math::Vec3 world_center(
        const wz::math::Mat4& world,
        const wz::math::Vec3& local_center)
    {
        return wz::math::mul_point(world, local_center);
    }

    inline bool needs_update(
        const TransformNode& node,
        const TransformNode* parent
    )
    {
        if (node.local_version != node.world_version)
            return true;

        if (parent && node.parent_version != parent->world_version)
            return true;

        return false;
    }

    void update_world(TransformNode* nodes, uint32_t id)
    {
        TransformNode& node = nodes[id];

        TransformNode* parent =
            node.parent != INVALID_TRANSFORM_NODE
            ? &nodes[node.parent]
            : nullptr;

        if (parent)
            update_world(nodes, node.parent);

        if (!needs_update(node, parent))
            return;

        const auto local = compute_local(node);

        if (parent)
            node.world = wz::math::mul(parent->world, local);
        else
            node.world = local;

        node.parent_version = parent ? parent->world_version : 0;
        node.world_version = node.local_version;
    }

    wz::math::Mat4 compute_local(const TransformNode& node)
    {
        return wz::math::transform(node.local);
    }

    wz::math::Mat4 compute_world(const TransformNode& node, const wz::math::Mat4& parent_world)
    {
        return wz::math::mul(parent_world, compute_local(node));
    }
}

