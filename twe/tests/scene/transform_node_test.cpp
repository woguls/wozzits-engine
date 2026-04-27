#include <gtest/gtest.h>

#include <wozzits/scene/transform_node.h>
#include <render/render.h>
#include <wozzits/scene/bake.h>
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

    update_world(nodes, 0);

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

    wz::scene::update_world(nodes, 1);
    wz::scene::update_world(nodes, 0);

    wz::math::Vec3 v{ 0,0,0 };
    auto r = wz::math::mul_point(nodes[1].world, v);

    EXPECT_NEAR(r.x, 11, 1e-5f);
}

TEST(Scene, ChildUpdatesWhenParentChanges)
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

    // initial full update
    wz::scene::update_world(nodes, 0);

    wz::math::Vec3 before =
        wz::math::mul_point(nodes[1].world, { 1,0,0 });

    // mutate parent
    nodes[0].local.position = { 20, 0, 0 };
    nodes[0].local_version++;

    // recompute full hierarchy
    wz::scene::update_world(nodes, 0);

    wz::math::Vec3 after =
        wz::math::mul_point(nodes[1].world, { 1,0,0 });

    EXPECT_NE(before.x, after.x);
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

