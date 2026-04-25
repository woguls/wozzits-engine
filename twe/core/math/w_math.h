#pragma once

namespace wz::math
{
    struct Vec3 { float x, y, z; };
    struct Vec4 { float x, y, z, w;  };

    struct Mat4 { float m[16]; };

    struct Quaternion{ float x, y, z, w; };

    struct Transform {
        Vec3 position;
        Quaternion rotation;
        Vec3 scale;
    };

    struct Plane {
        Vec3 normal; // plane.normal must always be unit length
        float distance; // plane constant: n·x + d = 0
    };

    struct Frustum {
        Plane planes[6]; // left, right, bottom, top, near, far
    };

    inline Vec3 to_view_space(
        const wz::math::Mat4& m,
        const wz::math::Vec3& p)
    {
        return {
            m.m[0] * p.x + m.m[4] * p.y + m.m[8] * p.z + m.m[12],
            m.m[1] * p.x + m.m[5] * p.y + m.m[9] * p.z + m.m[13],
            m.m[2] * p.x + m.m[6] * p.y + m.m[10] * p.z + m.m[14]
        };
    }

}