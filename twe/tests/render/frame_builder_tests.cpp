#include <gtest/gtest.h>

#include <render/frame.h>
#include <render/render.h>
#include <containers/buffer.h>
#include <wozzits/math/mat4.h>
#include <cmath>

using namespace wz;
using namespace wz::core::render;
using namespace wz::render;
using namespace wz::core::containers;

// ------------------------------------------------------------
// TEST HELPERS
// ------------------------------------------------------------

namespace {
    template<typename T, size_t N>
    Buffer<T> make_buffer(T(&storage)[N])
    {
        Buffer<T> b;
        b.data = storage;
        b.count = 0;
        b.capacity = N;
        return b;
    }
}

static RenderIR make_test_ir()
{
    RenderIR ir{};

    // -----------------------------
    // VIEW (identity camera at origin looking forward)
    // -----------------------------
    ViewData view{};

    view.view = wz::math::mat4_identity();   // IMPORTANT: must exist
    view.proj = wz::math::mat4_identity();   // temporary simplification

    static ViewData view_storage[1];
    view_storage[0] = view;

    ir.views.data = view_storage;
    ir.views.count = 1;
    ir.views.capacity = 1;

    // -----------------------------
    // OBJECTS
    // -----------------------------
    static ObjectData objects[3];

    objects[0].world = wz::math::translation({ 0, 0, 5 });
    objects[1].world = wz::math::translation({100, 0, 5 });
    objects[2].world = wz::math::translation({ 1, 1, 5 });

    objects[0].effect_mask = EffectBits::Outline;
    objects[1].effect_mask = EffectBits::Glow;
    objects[2].effect_mask = EffectBits::XRay;

    objects[0].render_flags = RenderFlags::CastShadow;
    objects[1].render_flags = RenderFlags::CastShadow;
    objects[2].render_flags = RenderFlags::CastShadow;

    ir.objects.data = objects;
    ir.objects.count = 3;
    ir.objects.capacity = 3;

    // -----------------------------
    // BOUNDS
    // -----------------------------
    static BoundsData bounds[3];

    bounds[0].center = { 0, 0, 5 };
    bounds[0].radius = 1;

    bounds[1].center = { 100, 0, 5 };
    bounds[1].radius = 1;

    bounds[2].center = { 1, 1, 5 };
    bounds[2].radius = 1;

    ir.bounds.data = bounds;
    ir.bounds.count = 3;
    ir.bounds.capacity = 3;

    // -----------------------------
    // PRIMITIVES
    // -----------------------------
    static PrimitiveData prims[3];

    prims[0] = { 0, 0, 0, 0, 0 };
    prims[1] = { 1, 0, 0, 1, 0 };
    prims[2] = { 2, 0, 0, 2, 0 };

    ir.primitives.data = prims;
    ir.primitives.count = 3;
    ir.primitives.capacity = 3;

    // -----------------------------
    // MATERIALS
    // -----------------------------
    static MaterialData mats[1];

    mats[0].pipeline_state_id = 1;
    mats[0].shader_id = 0;
    mats[0].texture_set = 0;

    ir.materials.data = mats;
    ir.materials.count = 1;
    ir.materials.capacity = 1;

    return ir;
}

static RenderIR make_smoke_ir()
{
    RenderIR ir{};

    // ------------------------------------------------------------
    // VIEW
    // ------------------------------------------------------------
    static ViewData views[1];
    views[0].view = wz::math::mat4_identity();
    views[0].proj = wz::math::mat4_identity();

    ir.views.data = views;
    ir.views.count = 1;
    ir.views.capacity = 1;

    // ------------------------------------------------------------
    // OBJECTS (WORLD SPACE ONLY)
    // ------------------------------------------------------------
    static ObjectData objects[1];

    objects[0].world = wz::math::translation({ 0, 0, 5 });
    objects[0].effect_mask = EffectBits::Outline;
    objects[0].render_flags = RenderFlags::CastShadow;

    ir.objects.data = objects;
    ir.objects.count = 1;
    ir.objects.capacity = 1;

    // ------------------------------------------------------------
    // BOUNDS (LOCAL SPACE)
    // ------------------------------------------------------------
    static BoundsData bounds[1];

    bounds[0].center = { 0, 0, 0 };
    bounds[0].radius = 1;

    ir.bounds.data = bounds;
    ir.bounds.count = 1;
    ir.bounds.capacity = 1;

    // ------------------------------------------------------------
    // PRIMITIVE
    // ------------------------------------------------------------
    static PrimitiveData prims[1];

    prims[0] = { 0, 0, 0, 0, 0 };

    ir.primitives.data = prims;
    ir.primitives.count = 1;
    ir.primitives.capacity = 1;

    // ------------------------------------------------------------
    // MATERIAL
    // ------------------------------------------------------------
    static MaterialData mats[1];

    mats[0].pipeline_state_id = 1;
    mats[0].shader_id = 0;
    mats[0].texture_set = 0;

    ir.materials.data = mats;
    ir.materials.count = 1;
    ir.materials.capacity = 1;

    return ir;
}

// ------------------------------------------------------------
// Minimal valid IR (no semantic correctness assumed)
// ------------------------------------------------------------

// ------------------------------------------------------------
// TESTS
// ------------------------------------------------------------

// ------------------------------------------------------------
// SMOKE TEST
// ------------------------------------------------------------
TEST(FrameBuilder, Smoke_NoCrash_ExecutesPipeline)
{
    RenderIR ir = make_smoke_ir();

    RenderFrame frame;
    FrameBuilder builder;

    static uint32_t visible_storage[16];
    static DrawRef draw_storage[16];

    frame.visible_primitives.data = visible_storage;
    frame.visible_primitives.capacity = 16;
    frame.visible_primitives.count = 0;

    frame.draws.data = draw_storage;
    frame.draws.capacity = 16;
    frame.draws.count = 0;

    // Execute full pipeline
    builder.cull(ir, frame);
    builder.classify(ir, frame);
    builder.sort(frame);
    builder.bin(ir, frame);

    // If we got here, pipeline is valid and linked
    SUCCEED();
}

TEST(FrameBuilder, Cull_SingleObjectVisible)
{
    RenderIR ir{};

    // -----------------------------
    // VIEW (identity camera at origin)
    // -----------------------------
    static ViewData views[1];
    views[0].view = wz::math::mat4_identity();
    views[0].proj = wz::math::mat4_identity();

    views[0].visible_range_start = 0;
    views[0].visible_range_count = 0;

    ir.views.data = views;
    ir.views.count = 1;
    ir.views.capacity = 1;

    // -----------------------------
    // OBJECT (in front of camera)
    // -----------------------------
    static ObjectData objects[1];
    objects[0].world = wz::math::translation({ 0, 0, -5 });

    objects[0].effect_mask = EffectBits::Outline;
    objects[0].render_flags = RenderFlags::CastShadow;

    ir.objects.data = objects;
    ir.objects.count = 1;
    ir.objects.capacity = 1;

    // -----------------------------
    // BOUNDS
    // -----------------------------
    static BoundsData bounds[1];
    bounds[0].center = { 0, 0, -5 };
    bounds[0].radius = 1;

    ir.bounds.data = bounds;
    ir.bounds.count = 1;
    ir.bounds.capacity = 1;

    // -----------------------------
    // PRIMITIVES
    // -----------------------------
    static PrimitiveData prims[1];
    prims[0] = { 0, 0, 0, 0, 0 };

    ir.primitives.data = prims;
    ir.primitives.count = 1;
    ir.primitives.capacity = 1;

    // -----------------------------
    // MATERIALS
    // -----------------------------
    static MaterialData mats[1];
    mats[0].pipeline_state_id = 1;

    ir.materials.data = mats;
    ir.materials.count = 1;
    ir.materials.capacity = 1;

    // -----------------------------
    // RUN
    // -----------------------------
    RenderFrame frame;
    FrameBuilder builder;

    static uint32_t visible[16];
    frame.visible_primitives.data = visible;
    frame.visible_primitives.capacity = 16;
    frame.visible_primitives.count = 0;

    builder.cull(ir, frame);

    // -----------------------------
    // ASSERTIONS
    // -----------------------------
    ASSERT_EQ(frame.visible_primitives.count, 1u);
    EXPECT_EQ(frame.visible_primitives.data[0], 0u);
}