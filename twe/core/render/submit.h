#pragma once
#include <render/render.h>
#include <wozzits/scene/scene.h>

namespace wz::core::render
{
    void submit(const RenderIR& ir);

    void bake_scene(
        const wz::scene::Scene& scene,
        RenderIR& out_ir
    );
}