#pragma once

#include <queue/mpsc_ring_buffer.h>

namespace wz::core
{
    template <typename T, size_t Capacity>
    class MPSCRingBuffer
    {
        static_assert(Capacity > 1, "RingBuffer capacity must be > 1");

    public:
        MPSCRingBuffer() = default;
        ~MPSCRingBuffer() = default;

        MPSCRingBuffer(const MPSCRingBuffer &) = delete;
        MPSCRingBuffer &operator=(const MPSCRingBuffer &) = delete;

        bool try_push(T value)
        {
            return impl.try_push(std::move(value));
        }

        bool try_pop(T &out)
        {
            return impl.try_pop(out);
        }

        bool empty() const
        {
            return impl.empty();
        }

        void clear()
        {
            impl.clear();
        }

    private:
        wz::core::internal::MPSCRingBuffer<T, Capacity> impl; // <-- this is the definition
    };
}