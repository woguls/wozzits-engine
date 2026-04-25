#include <gtest/gtest.h>

#include <render/frame.h>
#include <render/render.h>
#include <containers/buffer.h>

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
    // VIEW (simple forward frustum)
    // -----------------------------
    ViewData view{};
    view.view = wz::math::Mat4{};
    view.proj = wz::math::Mat4{};
    view.visible_range_start = 0;
    view.visible_range_count = 0;

    view.frustum.planes[0] = { {  1,  0,  0 }, 10.0f };
    view.frustum.planes[1] = { { -1,  0,  0 }, 10.0f };

    view.frustum.planes[2] = { {  0,  1,  0 }, 10.0f };
    view.frustum.planes[3] = { {  0, -1,  0 }, 10.0f };

    view.frustum.planes[4] = { {  0,  0,  1 }, 0.0f };
    view.frustum.planes[5] = { {  0,  0, -1 }, 20.0f };



    static ViewData view_storage[1];
    view_storage[0] = view;

    ir.views.data = view_storage;
    ir.views.count = 1;
    ir.views.capacity = 1;

    // -----------------------------
    // OBJECTS
    // -----------------------------
    static ObjectData objects[3];

    objects[0].transform.position = { 0, 0, 5 };
    objects[1].transform.position = { 100, 0, 5 };
    objects[2].transform.position = { 1, 1, 5 };

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
    // BOUNDS (simple spheres)
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

    // -----------------------------
    // LIGHTS (unused for now)
    // -----------------------------
    ir.lights.data = nullptr;
    ir.lights.count = 0;
    ir.lights.capacity = 0;

    return ir;
}

// ------------------------------------------------------------
// TESTS
// ------------------------------------------------------------

TEST(FrameBuilder, CullProducesVisibleSet)
{
    RenderIR ir = make_test_ir();

    RenderFrame frame;
    FrameBuilder builder;

    static uint32_t visible_storage[128];

    frame.visible_primitives.data = visible_storage;
    frame.visible_primitives.count = 0;
    frame.visible_primitives.capacity = 128;

    builder.cull(ir, frame);

    // expected: primitives 0 and 2 are inside
    ASSERT_EQ(frame.visible_primitives.count, 2u);
    EXPECT_EQ(frame.visible_primitives.data[0], 0);
    EXPECT_EQ(frame.visible_primitives.data[1], 2);
}

TEST(FrameBuilder, ClassifyProducesDraws)
{
    RenderIR ir = make_test_ir();

    PassNode pass_storage[16];

    RenderFrame frame{};
    static uint32_t visible_storage[128];

    frame.visible_primitives.data = visible_storage;
    frame.visible_primitives.count = 0;
    frame.visible_primitives.capacity = 128;

    static DrawRef draw_storage[128];
    frame.draws.data = draw_storage;
    frame.draws.count = 0;
    frame.draws.capacity = 128;

    // frame.visible_primitives = make_buffer(visible_storage);
    // frame.draws = make_buffer(draw_storage);
    frame.passes = make_buffer(pass_storage);

    FrameBuilder builder;

    builder.cull(ir, frame);
    builder.classify(ir, frame);

    ASSERT_EQ(frame.draws.count, 2u);

    // sanity: sort keys exist and differ deterministically
    EXPECT_NE(frame.draws.data[0].sort_key,
        frame.draws.data[1].sort_key);
}

TEST(FrameBuilder, SortOrdersDraws)
{
    RenderIR ir = make_test_ir();
    RenderFrame frame{};

    static uint32_t visible_storage[128];

    frame.visible_primitives.data = visible_storage;
    frame.visible_primitives.count = 0;
    frame.visible_primitives.capacity = 128;

    static DrawRef draw_storage[128];
    frame.draws.data = draw_storage;
    frame.draws.count = 0;
    frame.draws.capacity = 128;


    PassNode pass_storage[16];


    //frame.visible_primitives = make_buffer(visible_storage);
    //frame.draws = make_buffer(draw_storage);
    frame.passes = make_buffer(pass_storage);

    FrameBuilder builder;

    builder.cull(ir, frame);
    builder.classify(ir, frame);
    builder.sort(frame);

    for (uint32_t i = 1; i < frame.draws.count; i++)
    {
        EXPECT_LE(frame.draws.data[i - 1].sort_key,
            frame.draws.data[i].sort_key);
    }
}

TEST(FrameBuilder, BinProducesPasses)
{
    RenderIR ir = make_test_ir();
    RenderFrame frame{};
    static uint32_t visible_storage[128];

    frame.visible_primitives.data = visible_storage;
    frame.visible_primitives.count = 0;
    frame.visible_primitives.capacity = 128;

    static DrawRef draw_storage[128];
    frame.draws.data = draw_storage;
    frame.draws.count = 0;
    frame.draws.capacity = 128;
    PassNode pass_storage[16];


    //frame.visible_primitives = make_buffer(visible_storage);
    //frame.draws = make_buffer(draw_storage);
    frame.passes = make_buffer(pass_storage);

    FrameBuilder builder;

    builder.cull(ir, frame);
    builder.classify(ir, frame);
    builder.sort(frame);
    builder.bin(ir, frame);

    // at least one pass must exist
    ASSERT_GT(frame.passes.count, 0u);

    // passes must cover all draws
    uint32_t total = 0;
    for (uint32_t i = 0; i < frame.passes.count; i++)
    {
        total += frame.passes.data[i].draw_count;
    }

    EXPECT_EQ(total, frame.draws.count);
}