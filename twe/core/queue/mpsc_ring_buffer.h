#pragma once

#include <atomic>
#include <utilities/wozassert.h>

namespace wz::core::internal
{
    template <typename T, size_t Capacity>
    class MPSCRingBuffer
    {
        static_assert(Capacity > 1, "RingBuffer capacity must be > 1");

    public:
        bool try_push(T value)
        {
            size_t t = tail.load(std::memory_order_relaxed);

            for (;;)
            {
                size_t h = head.load(std::memory_order_acquire);

                if (t - h >= Capacity)
                    return false; // full

                if (tail.compare_exchange_weak(
                        t,
                        t + 1,
                        std::memory_order_acq_rel,
                        std::memory_order_relaxed))
                    break;
            }

            buffer[t % Capacity] = std::move(value);
            return true;
        }

        bool try_pop(T &out)
        {
            size_t h = head.load(std::memory_order_relaxed);
            size_t t = tail.load(std::memory_order_acquire);

            if (h == t)
            {
                // queue empty (not an error)
                return false;
            }

            out = std::move(buffer[h]);

            head.store(increment(h), std::memory_order_release);
            return true;
        }

        bool empty() const
        {
            return head.load(std::memory_order_acquire) ==
                   tail.load(std::memory_order_acquire);
        }

        void clear()
        {
            // safe only for consumer thread
            WZ_CORE_ASSERT(
                head.load() == tail.load() && "Clearing ring buffer while items remain");

            head.store(0);
            tail.store(0);
        }

    private:
        static constexpr size_t increment(size_t i)
        {
            return (i + 1) % Capacity;
        }

    private:
        alignas(64) std::atomic<size_t> head{0};
        alignas(64) std::atomic<size_t> tail{0};

        T buffer[Capacity];
    };
}