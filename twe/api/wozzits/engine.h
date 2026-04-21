#pragma once

#include <functional>
#include <cstdint>

namespace wz::engine
{
    /**
     * @brief Runtime state of the engine for a single frame.
     *
     * The Context object is passed to the user-defined update function
     * every frame and contains core execution state.
     *
     * @warning This structure currently mixes engine-internal state
     * and application-facing data. This is intentional for early design
     * simplicity but should be revisited.
     *
     * @todo Separate engine-internal state from application-facing frame data.
     * Consider introducing:
     * - EngineState (internal timing, scheduling, subsystem state)
     * - FrameData (delta time, frame index, input snapshot, etc.)
     *
     * @todo Define a formal "frame contract":
     * - What is guaranteed to be valid before update()
     * - What systems run before/after update()
     * - What data is stable during a frame
     */
    struct Context
    {
        /**
         * @brief Controls whether the engine loop continues running.
         *
         * Set to false to request shutdown at end of frame.
         */
        bool running = true;

        /**
         * @brief Current frame index since engine start.
         */
        uint64_t frame = 0;

        /**
         * @brief Time elapsed between the current and previous frame (seconds).
         */
        double delta_time = 0.0;
    };

    /**
     * @brief Function signature for per-frame update callbacks.
     *
     * @todo Consider replacing with a more flexible invocation model
     * (e.g. multiple stages: pre_update / update / post_update).
     */
    using UpdateFn = void (*)(Context &ctx, void *user_data);

    /**
     * @brief Starts the engine main loop.
     *
     * Runs until shutdown() is called.
     */
    void run(UpdateFn update, void *user_data);

    /**
     * @brief Requests engine shutdown.
     */
    void shutdown();

    /**
     * @brief Returns a reference to the global engine context.
     */
    Context &context();
}