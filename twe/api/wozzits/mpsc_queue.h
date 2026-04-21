#pragma once

#include <queue/mpsc_queue.h>

namespace wz::core
{
    template <typename T>
    class MPSCQueue
    {
    public:
        MPSCQueue() = default;
        ~MPSCQueue() = default;

        MPSCQueue(const MPSCQueue &) = delete;
        MPSCQueue &operator=(const MPSCQueue &) = delete;

        bool try_push(T value)
        {
            return impl.try_push(std::move(value));
        }

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
        wz::core::internal::MPSCQueue<T> impl; // <-- this is the definition
    };
}