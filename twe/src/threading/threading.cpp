#include <wozzits/threading.h>
#include <thread>
#include <chrono>

namespace WZ::Threading
{

    struct ThreadImpl
    {
        std::thread thread;
        bool joined_or_detached = false;
    };

    ThreadHandle create(ThreadFn fn, void *data)
    {
        ThreadImpl *impl = new ThreadImpl{};

        impl->thread = std::thread([fn, data]()
                                   { fn(data); });

        return ThreadHandle{impl};
    }

    void join(ThreadHandle &handle)
    {
        auto *impl = static_cast<ThreadImpl *>(handle.internal);

        if (!impl || impl->joined_or_detached)
            return;
        if (impl->thread.joinable())
            impl->thread.join();

        impl->joined_or_detached = true;
        delete impl;
        handle.internal = nullptr;
    }

    void detach(ThreadHandle &handle)
    {
        auto *impl = static_cast<ThreadImpl *>(handle.internal);
        if (!impl || impl->joined_or_detached)
            return;

        impl->thread.detach();
        impl->joined_or_detached = true;
        delete impl;
        handle.internal = nullptr;
    }

    void sleep_ms(uint32_t ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void yield()
    {
        std::this_thread::yield();
    }

}