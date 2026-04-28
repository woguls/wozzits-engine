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
        union
        {
            struct
            {
                Vec3 normal;   // x, y, z
                float distance; // w
            };

            Vec4 asVec4;
        };
    };

    struct Frustum
    {
        Plane planes[6];
    };

    struct Sphere
    {
        Vec3 center;
        float radius;
    };

    struct Camera
    {
        Transform transform;

        float fov_y;
        float near_plane;
        float far_plane;
        float aspect;
    };


}