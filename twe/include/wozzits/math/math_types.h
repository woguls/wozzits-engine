#pragma once

namespace wz::math
{
    struct Vec3
    {
        float x, y, z;
    };

    struct Vec4
    {
        float x, y, z, w;
    };

    struct Quaternion
    {
        float x, y, z, w;
    };

    struct Mat4
    {
        // column-major storage
        float m[16];
    };

    struct Transform
    {
        Vec3 position;
        Quaternion rotation;
        Vec3 scale;
    };

    struct Plane
    {
        Vec3 normal;   // must be normalized
        float distance;
    };

    struct Frustum
    {
        Plane planes[6];
    };
}