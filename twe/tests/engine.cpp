#include <gtest/gtest.h>
#include <wozzits/engine.h>

namespace EngineTest
{
    struct EngineTestHarness
    {
        uint64_t max_frames = 0;
        uint64_t frame_count = 0;

        bool shutdown_requested = false;

        std::function<void(wz::engine::Context &)> per_frame;
        std::function<void(wz::engine::Context &)> on_start;
        std::function<void(wz::engine::Context &)> on_end;

        static void per_frame_callback(wz::engine::Context &ctx, void *user)
        {
            auto *h = static_cast<EngineTestHarness *>(user);

            if (h->frame_count == 0 && h->on_start)
                h->on_start(ctx);

            if (h->per_frame)
                h->per_frame(ctx);

            h->frame_count++;

            if (h->frame_count >= h->max_frames || h->shutdown_requested)
                wz::engine::shutdown();
        }

        void run()
        {
            wz::engine::run(&per_frame_callback, this);

            if (on_end)
            {
                wz::engine::Context dummy;
                on_end(dummy);
            }
        }

        void stop()
        {
            shutdown_requested = true;
        }
    };
}

using namespace wz;

TEST(EngineSmokeTest, RunsForNFrames)
{
    EngineTest::EngineTestHarness h;

    h.max_frames = 10;

    bool callback_seen = false;

    h.per_frame = [&](engine::Context &ctx)
    {
        callback_seen = true;
    };

    h.run();

    // ✔ engine truth: harness is now authoritative for test logic
    EXPECT_EQ(h.frame_count, 10);
    EXPECT_TRUE(callback_seen);
}