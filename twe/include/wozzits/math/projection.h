#pragma once

#include "math_types.h"

namespace wz::math
{
    Mat4 projection_perspective(float fov_y_radians, float aspect, float near_z, float far_z);
}