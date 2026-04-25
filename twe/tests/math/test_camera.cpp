#include <gtest/gtest.h>
#include <wozzits/math/vec3.h>
#include <wozzits/math/quaternion.h>
#include <wozzits/math/mat4.h>

#include <wozzits/math/camera.h>
#include <wozzits/math/projection.h>
#include <wozzits/math/frustum.h>

using namespace wz::math;

TEST(CoordinateTransform, ObjectInFrontOfCamera)
{
    Transform camera;
    camera.position = { 0, 0, 0 };
    camera.rotation = quat_identity();
    camera.scale = { 1,1,1 };

    Transform obj;
    obj.position = { 0, 0, -5 };
    obj.rotation = quat_identity();
    obj.scale = { 1,1,1 };

    Mat4 V = view_matrix(camera);
    Mat4 W = transform(obj);

    Vec3 p = mul_point(mul(V, W), { 0,0,0 });

    EXPECT_LT(p.z, 0.0f); // in front in view space
}

TEST(CoordinateTransform, CameraLooksDownNegativeZ)
{
    Transform camera;
    camera.position = { 0,0,0 };
    camera.rotation = quat_identity();

    Mat4 V = view_matrix(camera);

    Vec3 forward_world = { 0,0,-1 };

    Vec3 forward_view = mul_vector(V, forward_world);

    EXPECT_NEAR(forward_view.z, -1.0f, 1e-5f);
}

TEST(CoordinateTransform, ProjectionDepthSign)
{
    Mat4 P = projection_perspective(1.0f, 1.0f, 0.1f, 100.0f);

    Vec4 front = vec4_mul_point(P, { 0, 0, -1 });
    Vec4 back = vec4_mul_point(P, { 0, 0,  1 });

    // perspective divide to compare depth meaningfully
    float front_ndc_z = front.z / front.w;
    float back_ndc_z = back.z / back.w;

    EXPECT_LT(front_ndc_z, back_ndc_z);
}

TEST(Frustum, PointInFrontIsInside)
{
    Mat4 V = view_matrix({ {0,0,0}, quat_identity(), {1,1,1} });
    Mat4 P = projection_perspective(1.0f, 1.0f, 0.1f, 100.0f);

    Frustum f = frustum_from_view_projection(mul(P, V));

    EXPECT_TRUE(contains_point(f, { 0, 0, -5 }));
}

TEST(Frustum, PointBehindIsOutside)
{
    Mat4 V = view_matrix({ {0,0,0}, quat_identity(), {1,1,1} });
    Mat4 P = projection_perspective(1.0f, 1.0f, 0.1f, 100.0f);

    Frustum f = frustum_from_view_projection(mul(P, V));

    EXPECT_FALSE(contains_point(f, { 0, 0, 5 }));
}

TEST(Frustum, FarPlaneRejects)
{
    Mat4 V = view_matrix({ {0,0,0}, quat_identity(), {1,1,1} });
    Mat4 P = projection_perspective(1.0f, 1.0f, 0.1f, 10.0f);

    Frustum f = frustum_from_view_projection(mul(P, V));

    EXPECT_FALSE(contains_point(f, { 0, 0, -50 }));
}

TEST(Frustum, SphereInside)
{
    Camera cam;
    cam.transform = { {0,0,0}, quat_identity(), {1,1,1} };
    cam.fov_y = 1.0f;
    cam.aspect = 1.0f;
    cam.near_plane = 0.1f;
    cam.far_plane = 100.0f;

    Mat4 V = view_matrix(cam);
    Mat4 P = projection_perspective(cam);

    Frustum f = frustum_from_view_projection(mul(P, V));

    EXPECT_TRUE(intersects_sphere(f, { 0,0,-5 }, 1.0f));
}

TEST(Frustum, SphereOutside)
{
    Camera cam;
    cam.transform = { {0,0,0}, quat_identity(), {1,1,1} };
    cam.fov_y = 1.0f;
    cam.aspect = 1.0f;
    cam.near_plane = 0.1f;
    cam.far_plane = 100.0f;

    Mat4 V = view_matrix(cam);
    Mat4 P = projection_perspective(cam);

    Frustum f = frustum_from_view_projection(mul(P, V));

    EXPECT_FALSE(intersects_sphere(f, { 0,0,5 }, 1.0f));
}