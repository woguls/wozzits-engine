#pragma once

#include <queue/mpsc_ring_buffer.h>

namespace wz::core
{
    /**
     * @brief Lock-free bounded multiple-producer single-consumer ring buffer.
     *
     * This container provides a fixed-capacity FIFO queue suitable for real-time systems
     * such as input/event dispatch, where predictable memory usage and bounded latency
     * are required.
     *
     * Design characteristics:
     * - Multiple producers (thread-safe)
     * - Single consumer
     * - Fixed capacity (no allocations)
     * - Non-blocking push (returns false when full)
     * - Non-blocking pop (returns false when empty)
     *
     * Backpressure behavior:
     * - When full, `try_push` returns false instead of blocking.
     * - It is the caller's responsibility to decide retry/backoff/drop policy.
     *
     * This makes the queue suitable for real-time engine subsystems where blocking
     * is undesirable (input, frame events, etc).
     *
     * ---
     *
     * @tparam T Type stored in the ring buffer
     * @tparam Capacity Maximum number of elements in the buffer (must be > 1)
     */
    template <typename T, size_t Capacity>
    class MPSCRingBuffer
    {
        static_assert(Capacity > 1, "RingBuffer capacity must be > 1");

    public:
        MPSCRingBuffer() = default;
        ~MPSCRingBuffer() = default;

        MPSCRingBuffer(const MPSCRingBuffer &) = delete;
        MPSCRingBuffer &operator=(const MPSCRingBuffer &) = delete;

        /**
         * @brief Attempts to push an element into the ring buffer.
         *
         * This operation is non-blocking:
         * - Returns true if the element was successfully enqueued.
         * - Returns false if the buffer is full.
         *
         * In a multi-producer context, this operation is thread-safe.
         *
         * @param value Element to enqueue (moved if possible)
         * @return true if pushed successfully
         * @return false if the buffer is full
         */
        bool try_push(T value)
        {
            return impl.try_push(std::move(value));
        }

        /**
         * @brief Attempts to pop an element from the ring buffer.
         *
         * This operation is non-blocking:
         * - Returns true if an element was available.
         * - Returns false if the buffer is empty.
         *
         * Only a single consumer thread must call this function.
         *
         * @param out Output parameter receiving the popped value
         * @return true if a value was popped
         * @return false if the buffer is empty
         */
        bool try_pop(T &out)
        {
            return impl.try_pop(out);
        }

        /**
         * @brief Checks whether the buffer is empty.
         *
         * Note: This is a snapshot in time and may become stale immediately
         * in concurrent contexts.
         *
         * @return true if empty, false otherwise
         */
        bool empty() const
        {
            return impl.empty();
        }

        /**
         * @brief Clears the buffer.
         *
         * Must only be called from the consumer thread while no producers are active.
         * Behavior under concurrent access is undefined.
         */
        void clear()
        {
            impl.clear();
        }

    private:
        wz::core::internal::MPSCRingBuffer<T, Capacity> impl;

        /*
        ============================================================================
        TODO: Future Engine Enhancements
        ============================================================================

        A) Diagnostics & Backpressure Telemetry
           - Track number of failed pushes
           - Track time spent spinning/yielding in producers
           - Expose "queue pressure" metric for engine profiling

        B) Two-Tier Overflow System (Hybrid Queue)
           - Primary: fast ring buffer
           - Secondary: fallback MPSC linked queue
           - Used when ring buffer saturates under burst load

        C) Adaptive Push Strategy (Smart Backoff)
           - spin → yield → sleep → drop policy
           - dynamically adjust based on contention level
           - optional "real-time mode" vs "throughput mode"
        */
    };
}