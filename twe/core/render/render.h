#pragma once
#include <math/w_math.h>
#include <cstdint>

namespace wz::core::render
{
    using ObjectID = uint32_t;
    using PrimitiveID = uint32_t;
    using MaterialID = uint32_t;
    using BoundsID = uint32_t;

    template<typename T>
    struct Buffer {
        T* data;
        uint32_t count;
    };
}

namespace wz::core::render
{
    struct ViewData {
        wz::math::Mat4 view;
        wz::math::Mat4 proj;
        wz::math::Frustum frustum;

        uint32_t visible_range_start;
        uint32_t visible_range_count;
    };

    struct BoundsData {
        wz::math::Vec3 center;
        float radius;
    };

    struct MaterialData {
        uint32_t shader_id;
        uint32_t pipeline_state;
        uint32_t texture_set;
    };

    struct LightData {
        wz::math::Vec3 position;
        float radius;

        wz::math::Vec3 direction;
        float spot_angle;

        wz::math::Vec3 color;
        float intensity;

        uint32_t type;
        uint32_t flags;
    };

    struct FrameData {
        float time;
        float delta_time;
        wz::math::Vec3 ambient;
        float exposure;
    };

    enum class LightingMode {
        Unlit,
        Forward,
        Deferred,
        TiledForward,
    };

    enum class RenderFlags : uint32_t {
        CastShadow = 1 << 0,
        ReceiveShadow = 1 << 1,
        WriteDepth = 1 << 2,
        DoubleSided = 1 << 3,
    };

    enum class EffectBits : uint32_t {
        Outline = 1 << 0,
        Glow = 1 << 1,
        XRay = 1 << 2,
    };

    enum class SortMode : uint32_t
    {
        None,
        FrontToBack,
        BackToFront,
        Material,
        Shader,
        Custom
    };

    enum class DispatchType : uint32_t
    {
        Direct,     // CPU submits draw calls
        Indirect,   // GPU-driven draw arguments
        Compute     // compute shader pass
    };

    enum class PassType : uint32_t
    {
        Opaque,
        Transparent,
        Shadow,
        PostProcess,
        Compute,
        UI
    };

    inline EffectBits operator|(EffectBits a, EffectBits b) {
        return (EffectBits)((uint32_t)a | (uint32_t)b);
    }

    inline EffectBits operator&(EffectBits a, EffectBits b) {
        return (EffectBits)((uint32_t)a & (uint32_t)b);
    }

    inline EffectBits operator~(EffectBits a) {
        return (EffectBits)(~(uint32_t)a);
    }

    inline EffectBits& operator|=(EffectBits& a, EffectBits b) {
        a = (EffectBits)((uint32_t)a | (uint32_t)b);
        return a;
    }
}

namespace wz::core::render
{
    struct ResourceBindings
    {
        uint32_t frame_uniform;   // camera, time, etc.

        uint32_t texture_start;
        uint32_t texture_count;

        uint32_t buffer_start;
        uint32_t buffer_count;

        uint32_t pipeline_state;
    };

    struct ObjectData {
        wz::math::Transform transform;

        EffectBits effect_mask;   // post / extra passes
        RenderFlags render_flags;  // pipeline behavior
    };

    struct PrimitiveData {
        ObjectID object_id;
        uint32_t mesh_id;
        MaterialID material_id;
        BoundsID bounds_id;
        uint32_t submesh_index;
    };

    struct DrawRef {
        uint64_t sort_key;
        uint32_t primitive_id;
        uint32_t pass_mask;
    };

    struct PassNode {
        PassType type;

        uint32_t draw_start;
        uint32_t draw_count;

        uint32_t pass_bit;

        SortMode sort_mode;
        DispatchType dispatch;

        LightingMode lighting;

        ResourceBindings resources;
    };
}

namespace wz::core::render
{
    struct RenderIR {
        // Views
        Buffer<ViewData> views;

        // Scene
        Buffer<ObjectData> objects;
        Buffer<PrimitiveData> primitives;
        Buffer<BoundsData> bounds;

        // Materials
        Buffer<MaterialData> materials;

        // Lighting
        Buffer<LightData> lights;

        // Draw indirection
        Buffer<DrawRef> draws;

        // Execution
        Buffer<PassNode> passes;

        FrameData frame;
    };
}