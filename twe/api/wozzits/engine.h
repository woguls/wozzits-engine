#pragma once

#include <functional>
#include <cstdint>

namespace wz::engine
{
    struct Context
    {
        bool running = true;
        uint64_t frame = 0;
        double delta_time = 0.0;
    };

    using UpdateFn = std::function<void(Context &)>;

    void run(UpdateFn update);
    void shutdown();

    Context &context();
}