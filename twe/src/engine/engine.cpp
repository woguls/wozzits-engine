#include <wozzits/engine.h>

#include <win32/win32.h>

#include <wozzits/w_time.h>
#include <wozzits/logger.h>

namespace wz::engine
{
    static Context g_ctx;

    Context &context()
    {
        return g_ctx;
    }

    void shutdown()
    {
        g_ctx.running = false;
    }

    void run(UpdateFn update, void *user_data)
    {
        using namespace wz::time;
        Context &ctx = context();

        uint64_t last = TimeSource::now_ticks();

        while (ctx.running)
        {
            platform::win32::w32_pump_messages();

            uint64_t now = TimeSource::now_ticks();
            uint64_t dt = now - last;
            last = now;

            ctx.delta_time = double(dt) / double(TimeSource::ticks_per_second());
            ctx.frame++;

            update(ctx, user_data);
        }
    }
}