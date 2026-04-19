#pragma once
#include <cstdint>

namespace WZ::Threading
{
    using ThreadFn = void (*)(void *);

    struct ThreadHandle
    {
        void *internal = nullptr;
    };

    ThreadHandle create(ThreadFn fn, void *data);
    void join(ThreadHandle &thread);
    void detach(ThreadHandle &thread);

    void sleep_ms(uint32_t ms);
    void yield();
}