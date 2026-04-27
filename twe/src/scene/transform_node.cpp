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

    void update_world_transform(TransformNode* nodes, uint32_t id)
    {
        TransformNode& node = nodes[id];

        TransformNode* parent =
            node.parent != INVALID_TRANSFORM_NODE
            ? &nodes[node.parent]
            : nullptr;

        // 1. ALWAYS ensure parent is up to date first
        if (parent)
            update_world_transform(nodes, node.parent);

        // 2. compute dirty purely from authoritative signals
        const bool dirty =
            node.local_version != node.world_version ||
            (parent && node.parent_version != parent->world_version);

        // 3. IMPORTANT FIX: if parent changed, ALWAYS force recompute
        if (parent && node.parent_version != parent->world_version)
        {
            // force invalidation regardless of local state
            node.world_version = node.local_version - 1;
        }

        if (!dirty)
            return;

        const auto local = compute_local(node);

        node.world =
            parent
            ? wz::math::mul(parent->world, local)
            : local;

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

