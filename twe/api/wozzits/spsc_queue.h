#pragma once

#include <queue/spsc_queue.h>

namespace WZ
{
    template <typename T>
    class SPSCQueue
    {
    public:
        explicit SPSCQueue(size_t capacity)
            : impl(capacity)
        {
        }

        SPSCQueue(const SPSCQueue &) = delete;
        SPSCQueue &operator=(const SPSCQueue &) = delete;

        bool push(T value)
        {
            return impl.push(std::move(value));
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
        WZ::core::SPSCQueue<T> impl;
    };
}