#include <gtest/gtest.h>

#include "../api/engine.h"

namespace EngineTest
{
    struct EngineStep
    {
        WZ::Engine::Context ctx;
    };

    struct EngineTestHarness
    {
        uint64_t max_frames = 0;
        uint64_t frame_count = 0;

        bool shutdown_requested = false;

        std::function<void(WZ::Engine::Context &)> per_frame;
        std::function<void(WZ::Engine::Context &)> on_start;
        std::function<void(WZ::Engine::Context &)> on_end;

        void run()
        {
            WZ::Engine::run([&](WZ::Engine::Context &ctx)
                            {
                if (frame_count == 0 && on_start)
                    on_start(ctx);

                if (per_frame)
                    per_frame(ctx);

                frame_count++;

                if (frame_count >= max_frames || shutdown_requested)
                    WZ::Engine::shutdown(); });

            if (on_end)
            {
                WZ::Engine::Context dummy;
                on_end(dummy);
            }
        }

        void stop()
        {
            shutdown_requested = true;
        }
    };
}

using namespace WZ;

TEST(EngineSmokeTest, RunsForNFrames)
{
    EngineTest::EngineTestHarness h;

    h.max_frames = 10;

    bool callback_seen = false;

    h.per_frame = [&](Engine::Context &ctx)
    {
        callback_seen = true;
    };

    h.run();

    EXPECT_EQ(h.frame_count, 10);
    EXPECT_TRUE(callback_seen);
}