#include "../engine.h"

#include "../../src/platform/win32.h"
#include "../wozzits_time.h"
#include "../logger.h"

namespace WZ::Engine
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
        using namespace WZ::Time;

        Logger logger;
        logger.set_callback(LogSinkType::Stderr);

        uint64_t last = Clock::now();

        logger.info("Engine started");

        while (g_ctx.running)
        {
            // 1. OS events
            WZ::platform::win32::w32_pump_messages();

            // 2. Timing
            uint64_t now = Clock::now();
            uint64_t dt = now - last;
            last = now;

            g_ctx.delta_time = dt * 1e-9;
            g_ctx.frame++;

            // 3. User update
            if (update)
                update(g_ctx);
        }

        logger.info("Engine shutting down");

        logger.flush();
    }
}