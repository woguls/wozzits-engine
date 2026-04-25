#pragma once

namespace wz::math
{

 /*
 
 1. inline everything hot

Functions like:

dot
cross
normalize
vec3 ops
mat4 mul (eventually)

become:

inline Vec3 dot(...)

👉 Goal: remove call overhead entirely
 
 ==================================================================================
 
 2. constexpr where valid
identity matrices
zero vectors
static constants
 


  ==================================================================================

  3. return-value optimization is assumed (but verified)

You structure so:

no unnecessary temporaries
no accidental copies in loop


  ==================================================================================


  Option B (engine-grade upgrade)

Introduce SoA or hybrid SIMD layout

Example:

struct Vec3SoA
{
    float* x;
    float* y;
    float* z;
};

or SIMD:

struct Vec3
{
    __m128 v; // x y z padding
};

👉 This is a major fork point
👉 You are NOT here yet

    ==================================================================================


🧭 Stage 3 — SIMD math layer (real performance jump)

This is where engines get fast.

What changes:
Vec3 becomes SIMD-friendly:
4-wide floats (SSE/AVX)
aligned memory
batch operations

Example:

Vec3 add(Vec3 a, Vec3 b) → _mm_add_ps
Mat4 becomes SIMD blocks:

Instead of scalar loops:

row vectors become SIMD rows
matrix multiply becomes 4 dot-products in parallel
Real impact:
3–10× speedup in math-heavy workloads
frustum checks become extremely cheap
skinning becomes viable



      ==================================================================================


🧭 Stage 4 — Batch processing mindset (engine shift)

At this point, math stops being “functions” and becomes:

data pipelines

You stop thinking:

Vec3 rotate(v, q)

You start thinking:

rotate_batch(Vec3* in, Vec3* out, Quaternion q, size_t count)
Why this matters

Modern engines win on:

cache coherence
batch transforms
SIMD lanes filled
minimizing branching

Not on individual function speed.


        ==================================================================================

🧭 Stage 5 — Structure of Arrays (advanced)

This is where high-end engines go.

Instead of:

struct Transform { Vec3 pos; Quaternion rot; };

You get:

pos_x[], pos_y[], pos_z[]
rot_x[], rot_y[], rot_z[], rot_w[]
Why:
GPU-like layout
perfect SIMD alignment
massive batch efficiency
Cost:
harder code
harder debugging
more complex API



        ==================================================================================
🧭 Stage 6 — Cache + pipeline awareness

At this level:

You optimize for:

L1 cache lines
branch prediction
instruction pairing
memory prefetching

Example:

frustum checks arranged in SoA
bounding volumes grouped spatially
traversal ordered for locality

     

 */

}