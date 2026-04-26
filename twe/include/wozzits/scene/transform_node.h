#pragma once
#include <cstdint>
#include <wozzits/math/mat4.h>

constexpr uint32_t INVALID_TRANSFORM_NODE = 0xFFFFFFFFu;

namespace wz::scene
{
    struct TransformNode
    {
        // authoritative
        wz::math::Transform local{};

        // hierarchy
        uint32_t parent = INVALID_TRANSFORM_NODE;

        // derived
        wz::math::Mat4 world{};

        // optimization
        uint32_t local_version = 1;      // increments when local changes
        uint32_t world_version = 0;      // last computed version
        uint32_t parent_version = 0;     // parent's version at last compute
    };

    inline bool needs_update(
        const TransformNode& node,
        const TransformNode* parent
    );

    void update_world(
        TransformNode* nodes,
        uint32_t id
    );
}

namespace wz::scene
{
    wz::math::Mat4 compute_local(const TransformNode& node);
    wz::math::Mat4 compute_world(const TransformNode& node, const wz::math::Mat4& parent_world);
    inline wz::math::Vec3 world_center(const wz::math::Mat4& world, const wz::math::Vec3& local_center);
}