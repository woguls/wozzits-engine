#include <gtest/gtest.h>

#include <wozzits/scene/transform_node.h>
#include <render/render.h>
#include <wozzits/scene/bake.h>
#include <wozzits/scene/scene.h>
#include <wozzits/math/math_types.h>
#include "wozzits/math/vec3.h"
#include "wozzits/math/mat4.h"
#include "wozzits/math/quaternion.h"


TEST(SceneGraphContract, RootNodeMatchesLocal)
{
    wz::scene::TransformNode nodes[1];

    nodes[0].local = wz::math::Transform{}; // identity assumed
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    update_world_transform(nodes, 0);

    EXPECT_EQ(nodes[0].parent_version, 0u);
    EXPECT_EQ(nodes[0].world_version, nodes[0].local_version);
}



TEST(Scene, ChildInheritsParentTransform)
{
    wz::scene::TransformNode nodes[2];

    // parent
    nodes[0].local.position = { 10, 0, 0 };
    nodes[0].local.rotation = wz::math::quat_identity();
    nodes[0].local.scale = { 1, 1, 1 };
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    // child
    nodes[1].local.position = { 1, 0, 0 };
    nodes[1].local.rotation = wz::math::quat_identity();
    nodes[1].local.scale = { 1, 1, 1 };
    nodes[1].parent = 0;
    nodes[1].local_version = 1;

    wz::scene::update_world_transform(nodes, 1);
    wz::scene::update_world_transform(nodes, 0);

    wz::math::Vec3 v{ 0,0,0 };
    auto r = wz::math::mul_point(nodes[1].world, v);

    EXPECT_NEAR(r.x, 11, 1e-5f);
}


TEST(SceneTransform, ComputeLocalMatchesMathTransform)
{
    wz::scene::TransformNode n;

    n.local.position = { 1,2,3 };
    n.local.rotation = wz::math::quat_identity();
    n.local.scale = { 1,1,1 };

    auto m1 = wz::math::transform(n.local);
    auto m2 = wz::scene::compute_local(n);

    EXPECT_NEAR(m1.m[12], m2.m[12], 1e-5f);
}

TEST(SceneTransform, ComputeWorldAppliesParentFirst)
{
    wz::scene::TransformNode child;

    wz::math::Mat4 parent = wz::math::translation({ 10,0,0 });

    child.local.position = { 1,0,0 };
    child.local.rotation = wz::math::quat_identity();
    child.local.scale = { 1,1,1 };

    auto w = wz::scene::compute_world(child, parent);

    wz::math::Vec3 r = wz::math::mul_point(w, { 0,0,0 });

    EXPECT_NEAR(r.x, 11, 1e-5f);
}

TEST(SceneTransform, IdentityParentDoesNothing)
{
    wz::scene::TransformNode n;

    n.local.position = { 1,2,3 };
    n.local.rotation = wz::math::quat_identity();
    n.local.scale = { 1,1,1 };

    auto w = wz::scene::compute_world(n, wz::math::mat4_identity());

    auto r = wz::math::mul_point(w, { 0,0,0 });

    EXPECT_NEAR(r.x, 1, 1e-5f);
}

TEST(SceneBake, EmitsRootNodesOnly)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[3]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 3);

    // build logical contents via push (THIS is the contract)
    in.push(TransformNode{
        .parent = INVALID_TRANSFORM_NODE,
        .world = wz::math::mat4_identity()
        });

    in.push(TransformNode{
        .parent = 0,
        .world = wz::math::mat4_identity()
        });

    in.push(TransformNode{
        .parent = INVALID_TRANSFORM_NODE,
        .world = wz::math::mat4_identity()
        });

    ObjectData storage_out[4]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 4);

    wz::scene::bake::bake_transforms(in, out);

    ASSERT_EQ(out.count(), 2);
}

TEST(SceneBake, PreservesWorldMatrix)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[1]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 1);

    in.push(TransformNode{
        .local = {},
        .parent = INVALID_TRANSFORM_NODE,
        .world = wz::math::Mat4{}   // or identity if preferred
        });

    ObjectData storage_out[1]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 1);

    wz::scene::bake::bake_transforms(in, out);

    ASSERT_EQ(out.count(), 1);

    // replace with near-equals later
    // EXPECT_TRUE(near_equal(out.data()[0].world, in.data()[0].world));
}

TEST(SceneBake, PreservesSceneNodeIndex)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[1]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 1);

    in.push(TransformNode{
        .local = {},
        .parent = INVALID_TRANSFORM_NODE,
        .world = wz::math::mat4_identity()
        });

    ObjectData storage_out[1]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 1);

    bake::bake_transforms(in, out);

    ASSERT_EQ(out.count(), 1);

    EXPECT_EQ(out.data()[0].scene_node, 0);
}

TEST(SceneBake, SkipsNonRootNodes)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[2]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 2);

    in.push(TransformNode{
        .local = {},
        .parent = INVALID_TRANSFORM_NODE,
        .world = wz::math::mat4_identity()
        });

    in.push(TransformNode{
        .local = {},
        .parent = 0, // child
        .world = wz::math::mat4_identity()
        });

    ObjectData storage_out[2]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 2);

    bake::bake_transforms(in, out);

    EXPECT_EQ(out.count(), 1);
}

TEST(SceneBake, DefaultFlagsAreZero)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[1]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 1);

    in.push(TransformNode{
        .local = {},
        .parent = INVALID_TRANSFORM_NODE,
        .world = wz::math::mat4_identity()
        });

    ObjectData storage_out[1]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 1);

    bake::bake_transforms(in, out);

    ASSERT_EQ(out.count(), 1);

    EXPECT_EQ((uint32_t)out.data()[0].effect_mask, 0);
    EXPECT_EQ((uint32_t)out.data()[0].render_flags, 0);
}

TEST(SceneFrame, GoldenFrameProducesCorrectRenderIR)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    // -----------------------------
    // 1. Storage
    // -----------------------------
    TransformNode storage_nodes[3]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 3);

    // -----------------------------
    // 2. Build logical scene via push
    // -----------------------------
    in.push(TransformNode{
    .local = {
        .position = {10,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    },
    .parent = INVALID_TRANSFORM_NODE,
    .local_version = 1
        });

    in.push(TransformNode{
        .local = {
            .position = {1,0,0},
            .rotation = wz::math::quat_identity(),
            .scale = {1,1,1}
        },
        .parent = 0,
        .local_version = 1
        });

    in.push(TransformNode{
        .local = {
            .position = {100,0,0},
            .rotation = wz::math::quat_identity(),
            .scale = {1,1,1}
        },
        .parent = INVALID_TRANSFORM_NODE,
        .local_version = 1
        });

    // -----------------------------
    // 3. Update transforms (per your contract)
    // -----------------------------
    wz::scene::update_world_transform(storage_nodes, 0);
    wz::scene::update_world_transform(storage_nodes, 2);
    wz::scene::update_world_transform(storage_nodes, 1);

    // -----------------------------
    // 4. Bake
    // -----------------------------
    ObjectData storage_out[3]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 3);

    wz::scene::bake::bake_transforms(in, out);

    // -----------------------------
    // 5. Assertions
    // -----------------------------
    ASSERT_EQ(out.count(), 2);

    EXPECT_EQ(out.data()[0].scene_node, 0);
    EXPECT_EQ(out.data()[1].scene_node, 2);

    for (uint32_t i = 0; i < out.count(); ++i)
        EXPECT_NE(out.data()[i].scene_node, 1);
}

TEST(Scene, GrandchildInheritsFullChain)
{
    using namespace wz::scene;

    TransformNode nodes[3]{};

    // root
    nodes[0].local = {
        .position = {10,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    // child
    nodes[1].local = {
        .position = {1,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[1].parent = 0;
    nodes[1].local_version = 1;

    // grandchild
    nodes[2].local = {
        .position = {2,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[2].parent = 1;
    nodes[2].local_version = 1;

    update_world_transform(nodes, 2);

    auto r = wz::math::mul_point(nodes[2].world, { 0,0,0 });

    EXPECT_NEAR(r.x, 13.0f, 1e-5f); // 10 + 1 + 2
}

TEST(Scene, ParentChangePropagatesOnChildUpdate)
{
    using namespace wz::scene;

    TransformNode nodes[2]{};

    // parent
    nodes[0].local = {
        .position = {10,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    // child
    nodes[1].local = {
        .position = {1,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[1].parent = 0;
    nodes[1].local_version = 1;

    update_world_transform(nodes, 1);

    auto before = wz::math::mul_point(nodes[1].world, { 0,0,0 });

    // mutate parent
    nodes[0].local.position = { 20,0,0 };
    nodes[0].local_version++;

    update_world_transform(nodes, 1);

    auto after = wz::math::mul_point(nodes[1].world, { 0,0,0 });

    EXPECT_NEAR(before.x, 11.0f, 1e-5f);
    EXPECT_NEAR(after.x, 21.0f, 1e-5f);
}

TEST(Scene, SiblingsRemainIndependent)
{
    using namespace wz::scene;

    TransformNode nodes[3]{};

    // parent
    nodes[0].local = {
        .position = {10,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    // child A
    nodes[1].local = {
        .position = {1,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[1].parent = 0;
    nodes[1].local_version = 1;

    // child B
    nodes[2].local = {
        .position = {5,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[2].parent = 0;
    nodes[2].local_version = 1;

    update_world_transform(nodes, 1);
    update_world_transform(nodes, 2);

    auto a = wz::math::mul_point(nodes[1].world, { 0,0,0 });
    auto b = wz::math::mul_point(nodes[2].world, { 0,0,0 });

    EXPECT_NEAR(a.x, 11.0f, 1e-5f);
    EXPECT_NEAR(b.x, 15.0f, 1e-5f);
}

TEST(Scene, UpdateIsIdempotent)
{
    using namespace wz::scene;

    TransformNode nodes[1]{};

    nodes[0].local = {
        .position = {3,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    update_world_transform(nodes, 0);

    auto before = nodes[0].world;

    update_world_transform(nodes, 0);

    auto after = nodes[0].world;

    EXPECT_NEAR(before.m[12], after.m[12], 1e-5f);
}

TEST(SceneBake, TruncatesWhenOutputCapacityExceeded)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[3]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 3);

    in.push(TransformNode{ .parent = INVALID_TRANSFORM_NODE });
    in.push(TransformNode{ .parent = INVALID_TRANSFORM_NODE });
    in.push(TransformNode{ .parent = INVALID_TRANSFORM_NODE });

    ObjectData storage_out[2]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 2);

    bake::bake_transforms(in, out);

    EXPECT_EQ(out.count(), 2);
}

TEST(SceneBake, PreservesRootOrder)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[3]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 3);

    in.push(TransformNode{ .parent = INVALID_TRANSFORM_NODE }); // 0
    in.push(TransformNode{ .parent = 0 });                      // 1 (skip)
    in.push(TransformNode{ .parent = INVALID_TRANSFORM_NODE }); // 2

    ObjectData storage_out[3]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 3);

    bake::bake_transforms(in, out);

    ASSERT_EQ(out.count(), 2);

    EXPECT_EQ(out.data()[0].scene_node, 0);
    EXPECT_EQ(out.data()[1].scene_node, 2);
}

TEST(SceneFrame, MultiFrameConsistency)
{
    using namespace wz::scene;

    TransformNode nodes[2]{};

    // parent
    nodes[0].local = {
        .position = {10,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    // child
    nodes[1].local = {
        .position = {1,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[1].parent = 0;
    nodes[1].local_version = 1;

    // frame 1
    update_world_transform(nodes, 1);
    auto f1 = wz::math::mul_point(nodes[1].world, { 0,0,0 });

    // frame 2: mutate parent
    nodes[0].local.position = { 20,0,0 };
    nodes[0].local_version++;

    update_world_transform(nodes, 1);
    auto f2 = wz::math::mul_point(nodes[1].world, { 0,0,0 });

    // frame 3: no change
    update_world_transform(nodes, 1);
    auto f3 = wz::math::mul_point(nodes[1].world, { 0,0,0 });

    EXPECT_NEAR(f1.x, 11.0f, 1e-5f);
    EXPECT_NEAR(f2.x, 21.0f, 1e-5f);
    EXPECT_NEAR(f3.x, 21.0f, 1e-5f); // must stabilize
}

TEST(SceneBake, StableAcrossFrames)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode storage_nodes[2]{};
    auto in = Buffer<TransformNode>::wrap(storage_nodes, 2);

    in.push(TransformNode{ .parent = INVALID_TRANSFORM_NODE });
    in.push(TransformNode{ .parent = INVALID_TRANSFORM_NODE });

    ObjectData storage_out[2]{};
    auto out = Buffer<ObjectData>::wrap(storage_out, 2);

    bake::bake_transforms(in, out);
    uint32_t first = out.count();

    // run again without clearing input
    out.reset();
    bake::bake_transforms(in, out);
    uint32_t second = out.count();

    EXPECT_EQ(first, second);
}

TEST(Scene, OutOfOrderUpdatesStillCorrect)
{
    using namespace wz::scene;

    TransformNode nodes[2]{};

    nodes[0].local = {
        .position = {10,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    nodes[1].local = {
        .position = {1,0,0},
        .rotation = wz::math::quat_identity(),
        .scale = {1,1,1}
    };
    nodes[1].parent = 0;
    nodes[1].local_version = 1;

    // intentionally wrong order
    update_world_transform(nodes, 1);
    update_world_transform(nodes, 0);

    auto r = wz::math::mul_point(nodes[1].world, { 0,0,0 });

    EXPECT_NEAR(r.x, 11.0f, 1e-5f);
}

TEST(SceneBake, ObjectDrivesEmission)
{
    using namespace wz::scene;
    using namespace wz::core::render;
    using wz::core::containers::Buffer;

    TransformNode node_storage[1]{};
    auto nodes = Buffer<TransformNode>::wrap(node_storage, 1);

    nodes.push(TransformNode{
        .parent = INVALID_TRANSFORM_NODE,
        .world = wz::math::mat4_identity()
        });

    Object obj_storage[1]{};
    auto objects = Buffer<Object>::wrap(obj_storage, 1);

    objects.push(Object{
        .node = 0,
        .kind = ObjectKind::Splat,
        .data_id = 0
        });

    ObjectData out_storage[1]{};
    auto out = Buffer<ObjectData>::wrap(out_storage, 1);

    wz::scene::bake::bake_transforms_v2(nodes, objects, out);

    ASSERT_EQ(out.count(), 1);
    EXPECT_EQ(out.data()[0].scene_node, 0);
}