#include <gtest/gtest.h>

#include <wozzits/engine.h>

namespace EngineTest
{
    struct EngineTestHarness; // forward declaration
    static EngineTestHarness *g_harness = nullptr;

    static void per_frame_callback(wz::engine::Context &ctx);

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
            g_harness = this;

            wz::engine::run(&per_frame_callback);

            g_harness = nullptr;

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

    static void per_frame_callback(wz::engine::Context &ctx)
    {
        auto &h = *g_harness;

        if (h.frame_count == 0 && h.on_start)
            h.on_start(ctx);

        if (h.per_frame)
            h.per_frame(ctx);

        h.frame_count++;

        if (h.frame_count >= h.max_frames || h.shutdown_requested)
            wz::engine::shutdown();
    }
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

    // ✔ Use engine truth, not harness bookkeeping
    EXPECT_EQ(wz::engine::context().frame, 10);
    EXPECT_TRUE(callback_seen);
}