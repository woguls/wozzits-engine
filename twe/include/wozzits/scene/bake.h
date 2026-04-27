#pragma once

#include <cstdint>

#include <containers/buffer.h>
#include <wozzits/math/mat4.h>

#include <wozzits/scene/transform_node.h>
#include <render/render.h>
#include <wozzits/scene/scene.h>

namespace wz::scene::bake
{
    void bake_transforms(
        const wz::core::containers::Buffer<wz::scene::TransformNode>& nodes,
        wz::core::containers::Buffer<wz::core::render::ObjectData>& objects);

    void bake_transforms_v2(
        const wz::core::containers::Buffer<TransformNode>& nodes,
        const wz::core::containers::Buffer<Object>& objects,
        wz::core::containers::Buffer<wz::core::render::ObjectData>& out);
}