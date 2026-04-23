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
        Vec3 normal;
        float distance; // signed distance from origin
    };

    struct Frustum {
        Plane planes[6]; // left, right, bottom, top, near, far
    };

}