#include <gtest/gtest.h>

#include <wozzits/scene/transform_node.h>
#include <wozzits/math/math_types.h>
#include "wozzits/math/vec3.h"
#include "wozzits/math/mat4.h"
#include "wozzits/math/quaternion.h"

TEST(Scene, RootNodeWorldMatchesLocal)
{
    wz::scene::TransformNode nodes[1];

    nodes[0].local.position = { 1, 2, 3 };
    nodes[0].local.rotation = wz::math::quat_identity();
    nodes[0].local.scale = { 1, 1, 1 };

    nodes[0].parent = INVALID_TRANSFORM_NODE;
    nodes[0].local_version = 1;

    wz::scene::update_world(nodes, 0);

    wz::math::Vec3 v{ 0,0,0 };
    auto r = wz::math::mul_point(nodes[0].world, v);

    EXPECT_NEAR(r.x, 1, 1e-5f);
    EXPECT_NEAR(r.y, 2, 1e-5f);
    EXPECT_NEAR(r.z, 3, 1e-5f);
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

    wz::scene::update_world(nodes, 1);
    wz::scene::update_world(nodes, 0);

    wz::math::Vec3 before = wz::math::mul_point(nodes[1].world, { 0,0,0 });

    // change parent
    nodes[0].local.position = { 20, 0, 0 };
    nodes[0].local_version++;

    wz::scene::update_world(nodes, 0);
    wz::scene::update_world(nodes, 1);

    wz::math::Vec3 after = wz::math::mul_point(nodes[1].world, { 0,0,0 });

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

