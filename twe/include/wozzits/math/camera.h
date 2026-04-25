#pragma once

#include "math_types.h"

namespace wz::math
{
    struct Camera
    {
        Transform transform;

        float fov_y;
        float near_plane;
        float far_plane;
        float aspect;
    };

    Mat4 view_matrix(const Transform& camera);
    Mat4 view_matrix(const Camera& camera);
    Mat4 projection_perspective(const Camera& camera);
}