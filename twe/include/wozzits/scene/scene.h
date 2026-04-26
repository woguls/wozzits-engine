#pragma once
#include <cstdint>
#include <wozzits/scene/transform_node.h>
#include <containers/buffer.h>

namespace wz::scene
{
    struct Scene
    {
        wz::core::containers::Buffer<TransformNode> nodes;
    };
}