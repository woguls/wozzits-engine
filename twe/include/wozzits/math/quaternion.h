#pragma once

#include <wozzits/math/math_types.h>

namespace wz::math
{
    Quaternion quat_identity();

    Quaternion normalize(const Quaternion& q);

    Quaternion mul(const Quaternion& a, const Quaternion& b);

    Quaternion from_axis_angle(const Vec3& axis, float angle);

    Vec3 rotate(const Quaternion& q, const Vec3& v);

    Vec4 vec4_mul_point(const Mat4& m, const Vec3& v);

    Mat4 to_mat4(const Quaternion& q);

    float dot(const Quaternion& a, const Quaternion& b);
}