#include <gtest/gtest.h>
#include <wozzits/math/mat4.h>
#include <wozzits/math/quaternion.h>

using namespace wz::math;

TEST(Mat4, Identity_PreservesPoint)
{
    Mat4 I = mat4_identity();

    Vec3 p{ 1, 2, 3 };

    Vec3 r = mul_point(I, p);

    EXPECT_FLOAT_EQ(r.x, 1);
    EXPECT_FLOAT_EQ(r.y, 2);
    EXPECT_FLOAT_EQ(r.z, 3);
}

TEST(Mat4, Translation_ShiftsPoint)
{
    Mat4 T = translation({ 10, 0, 0 });

    Vec3 p{ 1, 2, 3 };

    Vec3 r = mul_point(T, p);

    EXPECT_FLOAT_EQ(r.x, 11);
    EXPECT_FLOAT_EQ(r.y, 2);
    EXPECT_FLOAT_EQ(r.z, 3);
}

TEST(Mat4, Scale_ScalesPoint)
{
    Mat4 S = scale({ 2, 2, 2 });

    Vec3 p{ 1, 2, 3 };

    Vec3 r = mul_point(S, p);

    EXPECT_FLOAT_EQ(r.x, 2);
    EXPECT_FLOAT_EQ(r.y, 4);
    EXPECT_FLOAT_EQ(r.z, 6);
}

TEST(Mat4, RotationMatchesQuaternion)
{
    Vec3 v{ 1,0,0 };

    Quaternion q = from_axis_angle({ 0,0,1 }, 3.14159265f * 0.5f);

    Vec3 v1 = rotate(q, v);

    Mat4 R = rotation(q);
    Vec3 v2 = mul_vector(R, v);

    EXPECT_NEAR(v1.x, v2.x, 1e-5f);
    EXPECT_NEAR(v1.y, v2.y, 1e-5f);
    EXPECT_NEAR(v1.z, v2.z, 1e-5f);
}

TEST(Mat4, TransformComposition)
{
    Transform t;
    t.position = { 10,0,0 };
    t.scale = { 2,2,2 };
    t.rotation = quat_identity(); // quaternion identity

    Mat4 M = transform(t);

    Vec3 v{ 1,0,0 };
    Vec3 r = mul_point(M, v);

    // scale then translate → (1*2)+10 = 12
    EXPECT_FLOAT_EQ(r.x, 12);
}

TEST(Mat4, RotationZ90)
{
    Quaternion q = from_axis_angle({ 0,0,1 }, 3.14159265f * 0.5f);

    Mat4 R = rotation(q);

    Vec3 v{ 1,0,0 };
    Vec3 r = mul_vector(R, v);

    EXPECT_NEAR(r.x, 0.0f, 1e-5f);
    EXPECT_NEAR(r.y, 1.0f, 1e-5f);
}

