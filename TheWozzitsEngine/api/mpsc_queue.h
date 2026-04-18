#pragma once

#include "../src/core/mpsc_queue.h"

namespace WZ
{
    template <typename T>
    class MPSCQueue
    {
    public:
        MPSCQueue() = default;
        ~MPSCQueue() = default;

        MPSCQueue(const MPSCQueue &) = delete;
        MPSCQueue &operator=(const MPSCQueue &) = delete;

        void push(T value)
        {
            impl.push(std::move(value));
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
        WZ::core::MPSCQueue<T> impl; // <-- this is the definition
    };
}