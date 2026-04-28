#pragma once
#include <cstdint>
#include <containers/buffer.h>
#include <wozzits/math/mat4.h>

/**
 * @file render.h
 * @brief Core render intermediate representation (RenderIR) and pipeline contracts.
 *
 * This header defines the engine's CPU-side render submission model.
 *
 * It does NOT perform rendering. Instead, it describes:
 * - Scene data (objects, primitives, materials, lights)
 * - Frame-global state
 * - Render execution structure (passes, draw lists)
 * - Sorting and batching mechanisms
 *
 * RenderIR is consumed by FrameBuilder, which transforms it into
 * GPU-ready execution order.
 */

namespace wz::core::render
{
    using ObjectID = uint32_t;
    using PrimitiveID = uint32_t;
    using MaterialID = uint32_t;
    using BoundsID = uint32_t;
}

namespace wz::core::render
{

    /**
     * @brief Camera view state used for rendering.
     *
     * Contains both view/projection transforms and precomputed frustum
     * used for visibility culling.
     */
    struct ViewData {
        wz::math::Mat4 view;
        wz::math::Mat4 proj;

        wz::math::Frustum frustum;
    };

    /**
     * @brief Bounding sphere used for visibility and culling tests.
     * @todo replace with math::sphere
     */
    struct BoundsData {
        wz::math::Vec3 center;
        float radius;
    };

    /**
     * @brief GPU material descriptor.
     *
     * References shader, pipeline state, and texture bindings.
     */
    struct MaterialData {
        uint32_t shader_id;
        uint32_t pipeline_state_id;
        uint32_t texture_set;
    };

    /**
     * @brief Scene light source.
     *
     * Supports positional and directional lighting models depending on type.
     */
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

    /**
     * @brief Per-frame global simulation data.
     *
     * Shared across all render passes.
     */
    struct FrameData {
        float time;
        float delta_time;
        wz::math::Vec3 ambient;
        float exposure;
    };

    /**
     * @brief High-level lighting model used by a render pass.
     */
    enum class LightingMode {
        Unlit,
        Forward,
        Deferred,
        TiledForward,
    };

    /**
     * @brief Object-level render behavior flags.
     *
     * Controls how primitives participate in the rendering pipeline.
     */
    enum class RenderFlags : uint32_t {
        CastShadow = 1 << 0,
        ReceiveShadow = 1 << 1,
        WriteDepth = 1 << 2,
        DoubleSided = 1 << 3,
    };

    /**
     * @brief Per-object visual effect modifiers.
     *
     * Used to trigger additional rendering passes such as outlines or X-ray.
     */
    enum class EffectBits : uint32_t {
        Outline = 1 << 0,
        Glow = 1 << 1,
        XRay = 1 << 2,
    };

    /**
     * @brief Determines ordering strategy within a render pass.
     */
    enum class SortMode : uint32_t
    {
        None,
        FrontToBack,
        BackToFront,
        Material,
        Shader,
        Custom
    };

    /**
     * @brief Execution mode of a render pass.
     *
     * Direct: CPU submits draw calls.
     * Indirect: GPU-driven draw arguments.
     * Compute: compute shader-based execution.
     */
    enum class DispatchType : uint32_t
    {
        Direct,     // CPU submits draw calls
        Indirect,   // GPU-driven draw arguments
        Compute     // compute shader pass
    };

    /**
     * @brief High-level classification of a render pass.
     */
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
    /**
     * @brief GPU resource binding range for a render pass.
     *
     * Encapsulates descriptor ranges for textures, buffers, and pipelines.
     */
    struct ResourceBindings
    {
        uint32_t frame_uniform;   // camera, time, etc.

        uint32_t texture_start;
        uint32_t texture_count;

        uint32_t buffer_start;
        uint32_t buffer_count;

        uint32_t pipeline_state;
    };

    /**
     * @brief Scene object instance data.
     *
     * Represents transform and render behavior modifiers.
     */
    struct ObjectData {
        wz::math::Mat4 world; // to be removed after refactor and new object system
        uint32_t scene_node;

        EffectBits effect_mask;   // post / extra passes
        RenderFlags render_flags;  // pipeline behavior

        uint32_t bounds_id; // 
    };

    /**
     * @brief Renderable mesh instance referencing an object and material.
     */
    struct PrimitiveData {
        ObjectID object_id;
        uint32_t mesh_id;
        MaterialID material_id;
        BoundsID bounds_id;
        uint32_t submesh_index;
    };

    /**
     * @brief Defines bit layout of the 64-bit sort key.
     *
     * Sort keys are used to minimize GPU state changes by encoding
     * pipeline, material, and depth into a single sortable integer.
     *
     * Layout:
     * - PIPE     (16 bits)
     * - MATERIAL (16 bits)
     * - DEPTH    (24 bits)
     */
    struct SortKeyLayout
    {
        static constexpr uint64_t PIPE_SHIFT = 40;
        static constexpr uint64_t MATERIAL_SHIFT = 24;
        static constexpr uint64_t DEPTH_SHIFT = 0;

        static constexpr uint64_t PIPE_MASK = 0xFFFFull;
        static constexpr uint64_t MATERIAL_MASK = 0xFFFFull;
        static constexpr uint64_t DEPTH_MASK = 0xFFFFFFull;
    };


    // NOTE:
    // sort_key is a GPU state minimization heuristic, NOT a semantic identifier.
    // It must not encode scene hierarchy or gameplay state.
    /**
     * @brief Encodes GPU sorting state into a 64-bit key.
     *
     * @param pipe Pipeline or shader state identifier.
     * @param material Material identifier.
     * @param depth Quantized depth value (0–2^24).
     *
     * @return Packed sort key used for draw ordering.
     */
    inline uint64_t make_sort_key(
        uint32_t pipe,
        uint32_t material,
        uint32_t depth
    )
    {
        pipe &= SortKeyLayout::PIPE_MASK;
        material &= SortKeyLayout::MATERIAL_MASK;
        depth &= SortKeyLayout::DEPTH_MASK;

        return  (uint64_t(pipe) << SortKeyLayout::PIPE_SHIFT) |
                (uint64_t(material) << SortKeyLayout::MATERIAL_SHIFT) |
                (uint64_t(depth) << SortKeyLayout::DEPTH_SHIFT);
    }

    /**
     * @brief Renderable draw command reference.
     *
     * Contains sorting information and links to primitive data.
     *
     * pass_mask determines which render passes this draw participates in.
     */
    struct DrawRef {
        uint64_t sort_key;
        uint32_t primitive_id;
        uint32_t pass_mask;
    };

    /**
     * @brief Execution node representing a render pass.
     *
     * Each pass defines:
     * - which draws are included (via pass_mask filtering)
     * - how they are sorted
     * - how they are executed (graphics/compute/indirect)
     */
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
    /**
     * @brief Complete CPU-side render submission intermediate representation.
     *
     * RenderIR represents a fully assembled frame before GPU execution.
     *
     * It contains:
     * - Scene data (objects, primitives, bounds)
     * - Materials and lights
     * - Draw list (generated by FrameBuilder)
     * - Render pass graph (execution structure)
     *
     * It is mutable during FrameBuilder execution and should be considered
     * read-only afterward.
     */
    struct RenderIR {
        // Views
        wz::core::containers::Buffer<ViewData> views;

        // Scene
        wz::core::containers::Buffer<ObjectData> objects;
        wz::core::containers::Buffer<PrimitiveData> primitives;
        wz::core::containers::Buffer<BoundsData> bounds;

        // Materials
        wz::core::containers::Buffer<MaterialData> materials;

        // Lighting
        wz::core::containers::Buffer<LightData> lights;

        // Draw indirection
        wz::core::containers::Buffer<DrawRef> draws;

        // Execution
        wz::core::containers::Buffer<PassNode> passes;

        FrameData frame;
    };
}