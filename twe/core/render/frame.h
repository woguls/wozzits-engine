#pragma once
#include <render/render.h>
#include <containers/buffer.h>

namespace wz::render
{


    struct RenderFrame
    {
        // TODO: introducing a FrameArena so these Buffers are just views into a single per-frame memory slab
        wz::core::containers::Buffer<wz::core::render::DrawRef> draws;
        wz::core::containers::Buffer<wz::core::render::PassNode> passes;
        wz::core::containers::Buffer<uint32_t> visible_primitives;
        uint32_t pass_mask;
    };

    struct FrameBuilder
    {
        void build(
            const wz::core::render::RenderIR& ir,
            RenderFrame& out_frame
        );


        wz::core::containers::Buffer<wz::core::render::DrawRef> temp_draws;

        void cull(
            const wz::core::render::RenderIR& ir,
            RenderFrame& out_frame
        );

        void classify(
            const wz::core::render::RenderIR& ir,
            RenderFrame& out_frame
        );

        void sort(RenderFrame& out_frame);

        void bin(
            const wz::core::render::RenderIR& ir,
            RenderFrame& out_frame
        );
    };
}