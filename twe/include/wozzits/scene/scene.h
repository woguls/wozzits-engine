#pragma once
#include <cstdint>
#include <wozzits/scene/transform_node.h>
#include <containers/buffer.h>

namespace wz::scene
{
    using ObjectID = uint32_t;
    using NodeID = uint32_t;

    enum class ObjectKind : uint32_t
    {
        Mesh,       // traditional triangle mesh
        Splat       // gaussian splat (your first target)
    };

    struct Object
    {
        NodeID node;        // where it lives

        ObjectKind kind;    // what it is

        uint32_t data_id;   // index into a type-specific buffer

        uint32_t bounds_id;
    };

    struct Scene
    {
        wz::core::containers::Buffer<TransformNode> nodes;

        wz::core::containers::Buffer<NodeID> root_nodes;

        wz::core::containers::Buffer<Object> objects;

        // future (but you can stub now if you want)
        wz::core::containers::Buffer<uint32_t> splat_data;   // placeholder
    };
}