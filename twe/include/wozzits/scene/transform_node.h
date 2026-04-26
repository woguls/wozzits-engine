#pragma once
#include <cstdint>
#include <wozzits/math/mat4.h>

constexpr uint32_t INVALID_TRANSFORM_NODE = 0xFFFFFFFFu;

namespace wz::scene
{
    /**
     * @brief A single node in a hierarchical transform graph.
     *
     * TransformNodes form a directed acyclic graph (DAG) representing spatial
     * relationships. Each node stores a local transform (authoritative) and a
     * cached world-space transform derived from its parent chain.
     *
     * The system uses versioning to avoid recomputing world transforms when
     * neither the node nor its ancestry has changed.
     */
    struct TransformNode
    {
        /**
         * @brief Authoritative local-space transform.
         *
         * This is the only field that should be modified externally.
         * All derived state is recomputed from this value.
         */
        wz::math::Transform local{};

        /**
         * @brief Index of the parent node in the transform array.
         *
         * INVALID_TRANSFORM_NODE indicates a root node.
         *
         * The parent defines the coordinate space in which this node's local
         * transform is expressed.
         */
        uint32_t parent = INVALID_TRANSFORM_NODE;

        /**
         * @brief Cached world-space transform.
         *
         * This value is derived from the local transform and all parent
         * transforms. It is only valid when world_version is in sync with
         * local_version and parent_version.
         */
        wz::math::Mat4 world{};

        /**
         * @brief Version counter incremented when local transform changes.
         *
         * This acts as the "dirty signal" for the node. Any modification to
         * `local` must increment this value.
         */
        uint32_t local_version = 1;

        /**
         * @brief Version of the local transform at the time `world` was last computed.
         *
         * If this does not match `local_version`, the cached world transform is stale.
         */
        uint32_t world_version = 0;

        /**
         * @brief Snapshot of the parent's world_version used during last update.
         *
         * If this differs from the current parent's world_version, this node's
         * cached world transform is invalid and must be recomputed.
         */
        uint32_t parent_version = 0;
    };

    /**
     * @brief Determines whether a node's world transform is stale.
     *
     * A node requires recomputation if:
     * - Its local transform has changed since last evaluation, OR
     * - Its parent has changed in a way that affects world space.
     *
     * @param node   Node being evaluated.
     * @param parent Parent node (or nullptr if root).
     *
     * @return true if world transform must be recomputed.
     */
    inline bool needs_update(
        const TransformNode& node,
        const TransformNode* parent
    );

    /**
     * @brief Ensures that a node's world transform is up-to-date.
     *
     * This function performs a recursive traversal to guarantee that the
     * parent transform is updated before computing the child transform.
     *
     * The function is safe to call on any node; it will only recompute
     * transforms when versioning indicates invalidation.
     *
     * @param nodes Array of all transform nodes.
     * @param id    Index of the node to update.
     */
    void update_world(
        TransformNode* nodes,
        uint32_t id
    );
}

namespace wz::scene
{
    /**
     * @brief Converts a local transform into a matrix form.
     *
     * This is the lowest-level transform conversion step.
     *
     * @param node Node containing the local transform.
     * @return 4x4 transformation matrix representing local space.
     */
    wz::math::Mat4 compute_local(const TransformNode& node);

    /**
     * @brief Composes a world transform from parent and local transforms.
     *
     * @param node          Node containing local transform.
     * @param parent_world   Parent's world-space transform.
     *
     * @return Combined world-space transform.
     */
    wz::math::Mat4 compute_world(
        const TransformNode& node,
        const wz::math::Mat4& parent_world
    );

    /**
     * @brief Transforms a local-space center point into world space.
     *
     * Typically used for bounding volumes, pivot points, or debug rendering.
     *
     * @param world        World transformation matrix.
     * @param local_center Point in local space.
     *
     * @return Point transformed into world space.
     */
    inline wz::math::Vec3 world_center(
        const wz::math::Mat4& world,
        const wz::math::Vec3& local_center
    );
}