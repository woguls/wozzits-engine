#pragma once

#include "math_types.h"

namespace wz::math
{


    Mat4 view_matrix(const Transform& camera);
    Mat4 view_matrix(const Camera& camera);
    Mat4 projection_perspective(const Camera& camera);
}