#include <wozzits/math/quaternion.h>
#include <wozzits/math/vec3.h>
#include <cmath>

namespace wz::math
{
    Quaternion quat_identity()
    {
        return { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    Quaternion normalize(const Quaternion& q)
    {
        float len = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        if (len == 0.0f)
            return quat_identity();

        float inv = 1.0f / len;

        return {
            q.x * inv,
            q.y * inv,
            q.z * inv,
            q.w * inv
        };
    }

    Quaternion mul(const Quaternion& a, const Quaternion& b)
    {
        // Hamilton product
        return {
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
        };
    }

    Quaternion from_axis_angle(const Vec3& axis, float angle)
    {
        Vec3 n = normalize(axis);  // uses Vec3 normalize

        float half = angle * 0.5f;
        float s = std::sin(half);
        float c = std::cos(half);

        return {
            n.x * s,
            n.y * s,
            n.z * s,
            c
        };
    }

    Vec3 rotate(const Quaternion& q, const Vec3& v)
    {
        // Convert vector to quaternion (w = 0)
        Quaternion p{ v.x, v.y, v.z, 0.0f };

        // q * p * q^-1
        Quaternion qn = normalize(q);

        Quaternion q_conj{
            -qn.x,
            -qn.y,
            -qn.z,
             qn.w
        };

        Quaternion r = mul(mul(qn, p), q_conj);

        return { r.x, r.y, r.z };
    }

    Vec4 vec4_mul_point(const Mat4& m, const Vec3& v)
    {
        return {
            m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12],
            m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13],
            m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14],
            m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15]
        };
    }

    float dot(const Quaternion& a, const Quaternion& b)
    {
        return a.x * b.x +
            a.y * b.y +
            a.z * b.z +
            a.w * b.w;
    }
}