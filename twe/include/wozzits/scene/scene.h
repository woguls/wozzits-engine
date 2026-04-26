#pragma once
#include <cstdint>
#include <wozzits/scene/transform_node.h>
#include <containers/buffer.h>

namespace wz::scene
{
    using ObjectID = uint32_t;
    using NodeID = uint32_t;

    struct Object
    {
        NodeID node;   // the transform this object lives at
    };

    struct Scene
    {
        wz::core::containers::Buffer<TransformNode> nodes;

        wz::core::containers::Buffer<NodeID> root_nodes;
    };
}