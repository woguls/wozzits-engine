#pragma once
#include <render/render.h>

namespace wz::render
{
    template<typename T>
    struct Buffer {
        T* data;
        uint32_t count;
    };

    struct RenderFrame
    {
        Buffer<wz::core::render::DrawReff> draws;
        Buffer<wz::core::render::PassNode> passes;
        Buffer<uint32_t> visible_primitives;
        uint32_t pass_mask;
    };

    struct FrameBuilder
    {
        void build(
            const wz::core::render::RenderIR & ir,
            RenderFrame& out_frame
        );
    };
}