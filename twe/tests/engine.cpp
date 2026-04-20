#include <gtest/gtest.h>

#include <wozzits/engine.h>

namespace EngineTest
{
    struct EngineStep
    {
        wz::engine::Context ctx;
    };

    struct EngineTestHarness
    {
        uint64_t max_frames = 0;
        uint64_t frame_count = 0;

        bool shutdown_requested = false;

        std::function<void(wz::engine::Context &)> per_frame;
        std::function<void(wz::engine::Context &)> on_start;
        std::function<void(wz::engine::Context &)> on_end;

        void run()
        {
            wz::engine::run([&](wz::engine::Context &ctx)
                            {
                if (frame_count == 0 && on_start)
                    on_start(ctx);

                if (per_frame)
                    per_frame(ctx);

                frame_count++;

                if (frame_count >= max_frames || shutdown_requested)
                    wz::engine::shutdown(); });

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

    EXPECT_EQ(h.frame_count, 10);
    EXPECT_TRUE(callback_seen);
}