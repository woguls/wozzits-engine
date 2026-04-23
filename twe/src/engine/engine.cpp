#include <wozzits/engine.h>

#include <win32/win32.h>

#include <wozzits/w_time.h>
#include <wozzits/logger.h>
#include <wozzits/input.h>
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
        ctx.running = true;

        const double seconds_per_tick =
            1.0 / double(TimeSource::ticks_per_second());

        Tick last = TimeSource::now_ticks();
        uint64_t frame_index = 0;

        while (ctx.running)
        {
            platform::win32::w32_pump_messages();

            Tick now = TimeSource::now_ticks();

            Tick dt = (now > last) ? (now - last) : 1;
            Tick end = last + dt;

            FrameContext fctx;

            fctx.frame.index = frame_index++;
            fctx.frame.interval.start = last;
            fctx.frame.interval.end = end;

            fctx.delta_time = dt * seconds_per_tick;

            std::vector<wz::event::Event> frame_events;
            frame_events.reserve(4096);

            wz::event::Event e;
            while (wz::event::event_queue.try_pop(e))
            {
                frame_events.push_back(std::move(e));
            }

            wz::input::InputState input{};
            wz::input::build_input(input,
                frame_events.data(),
                frame_events.size(),
                fctx.frame);

            last = end;

            update(ctx, fctx, user_data);
        }
    }
}