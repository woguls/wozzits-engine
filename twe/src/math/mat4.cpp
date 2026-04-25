#include <wozzits/math/mat4.h>
#include <wozzits/math/quaternion.h>
#include <wozzits/math/camera.h>
#include <wozzits/math/vec3.h>
#include <cmath>

namespace wz::math
{
    Mat4 projection_perspective(float fov_y, float aspect, float near_z, float far_z)
    {
        float f = 1.0f / std::tan(fov_y * 0.5f);

        Mat4 m = {};

        m.m[0] = f / aspect;
        m.m[5] = f;
        m.m[10] = (far_z + near_z) / (near_z - far_z);
        m.m[11] = -1.0f;
        m.m[14] = (2.0f * far_z * near_z) / (near_z - far_z);

        return m;
    }

    Mat4 projection_perspective(const Camera& camera)
    {
        float f = 1.0f / std::tan(camera.fov_y * 0.5f);

        Mat4 m = {};

        m.m[0] = f / camera.aspect;
        m.m[5] = f;

        m.m[10] = (camera.far_plane + camera.near_plane) /
            (camera.near_plane - camera.far_plane);

        m.m[11] = -1.0f;

        m.m[14] = (2.0f * camera.far_plane * camera.near_plane) /
            (camera.near_plane - camera.far_plane);

        return m;
    }

    Mat4 view_matrix(const Transform& camera)
    {
        Quaternion qinv = {
            -camera.rotation.x,
            -camera.rotation.y,
            -camera.rotation.z,
             camera.rotation.w
        };

        Vec3 neg_pos = camera.position * -1.0f;

        Mat4 R = rotation(qinv);
        Mat4 T = translation(rotate(qinv, neg_pos));

        return mul(R, T);
    }

    Mat4 view_matrix(const Camera& camera)
    {
        const Transform& t = camera.transform;

        Quaternion qinv = {
            -t.rotation.x,
            -t.rotation.y,
            -t.rotation.z,
             t.rotation.w
        };

        Vec3 neg_pos = t.position * -1.0f;

        Mat4 R = rotation(qinv);
        Mat4 T = translation(rotate(qinv, neg_pos));

        return mul(R, T);
    }


    Mat4 mat4_identity()
    {
        Mat4 r = {};

        r.m[0] = 1.0f;
        r.m[5] = 1.0f;
        r.m[10] = 1.0f;
        r.m[15] = 1.0f;

        return r;
    }

    Mat4 translation(const Vec3& t)
    {
        Mat4 r = mat4_identity();

        // last column (column-major)
        r.m[12] = t.x;
        r.m[13] = t.y;
        r.m[14] = t.z;

        return r;
    }

    Mat4 scale(const Vec3& s)
    {
        Mat4 r = {};

        r.m[0] = s.x;
        r.m[5] = s.y;
        r.m[10] = s.z;
        r.m[15] = 1.0f;

        return r;
    }

    Mat4 mul(const Mat4& a, const Mat4& b)
    {
        Mat4 r = {};

        // Column-major multiplication: r = a * b
        // r(col, row) = sum_k a(k, row) * b(col, k)

        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                r.m[col * 4 + row] =
                    a.m[0 * 4 + row] * b.m[col * 4 + 0] +
                    a.m[1 * 4 + row] * b.m[col * 4 + 1] +
                    a.m[2 * 4 + row] * b.m[col * 4 + 2] +
                    a.m[3 * 4 + row] * b.m[col * 4 + 3];
            }
        }

        return r;
    }

    Vec3 mul_point(const Mat4& m, const Vec3& v)
    {
        // w = 1
        return {
            m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12],
            m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13],
            m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14]
        };
    }

    Vec3 mul_vector(const Mat4& m, const Vec3& v)
    {
        // w = 0 (no translation)
        return {
            m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z,
            m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z,
            m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z
        };
    }

    Mat4 rotation(const Quaternion& q)
    {
        Quaternion n = normalize(q);

        float x = n.x;
        float y = n.y;
        float z = n.z;
        float w = n.w;

        float xx = x * x;
        float yy = y * y;
        float zz = z * z;

        float xy = x * y;
        float xz = x * z;
        float yz = y * z;

        float wx = w * x;
        float wy = w * y;
        float wz = w * z;

        Mat4 m = {};

        // column-major layout
        m.m[0] = 1.0f - 2.0f * (yy + zz);
        m.m[1] = 2.0f * (xy + wz);
        m.m[2] = 2.0f * (xz - wy);
        m.m[3] = 0.0f;

        m.m[4] = 2.0f * (xy - wz);
        m.m[5] = 1.0f - 2.0f * (xx + zz);
        m.m[6] = 2.0f * (yz + wx);
        m.m[7] = 0.0f;

        m.m[8] = 2.0f * (xz + wy);
        m.m[9] = 2.0f * (yz - wx);
        m.m[10] = 1.0f - 2.0f * (xx + yy);
        m.m[11] = 0.0f;

        m.m[12] = 0.0f;
        m.m[13] = 0.0f;
        m.m[14] = 0.0f;
        m.m[15] = 1.0f;

        return m;
    }

    Mat4 transform(const Transform& t)
    {
        Mat4 S = scale(t.scale);
        Mat4 R = rotation(t.rotation);
        Mat4 T = translation(t.position);

        // apply S → R → T
        return mul(T, mul(R, S));
    }
}