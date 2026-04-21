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

    void run(UpdateFn update)
    {
        using namespace wz::time;

        Logger logger;
        logger.set_callback(LogSinkType::Stderr);

        uint64_t last = wz::time::TimeSource::now_ticks();

        logger.info("Engine started");

        while (g_ctx.running)
        {
            // 1. OS events
            wz::platform::win32::w32_pump_messages();

            // 2. Timing
            uint64_t now_ticks = wz::time::TimeSource::now_ticks();
            uint64_t dt = now_ticks - last;
            last = now_ticks;

            g_ctx.delta_time =
                double(dt) / double(wz::time::TimeSource::ticks_per_second());
            g_ctx.frame++;

            // 3. User update
            if (update)
                update(g_ctx);
        }

        logger.info("Engine shutting down");

        logger.flush();
    }
}