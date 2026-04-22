#include <gtest/gtest.h>
#include <wozzits/engine.h>

namespace EngineTest
{
    struct EngineTestHarness
    {
        uint64_t max_frames = 0;
        uint64_t frame_count = 0;

        bool shutdown_requested = false;

        std::function<void(wz::engine::Context &, wz::engine::FrameContext &)> per_frame;
        std::function<void(wz::engine::Context &, wz::engine::FrameContext &)> on_start;
        std::function<void(wz::engine::Context &, wz::engine::FrameContext &)> on_end;
        wz::engine::FrameContext last_frame{};

        static void per_frame_callback(
            wz::engine::Context &ctx,
            wz::engine::FrameContext &fctx,
            void *user)
        {
            auto *h = static_cast<EngineTestHarness *>(user);

            if (h->frame_count == 0 && h->on_start)
                h->on_start(ctx, fctx);

            if (h->per_frame)
                h->per_frame(ctx, fctx);

            h->frame_count++;

            h->last_frame = fctx;
            if (h->frame_count >= h->max_frames || h->shutdown_requested)
                wz::engine::shutdown();
        }

        void run()
        {
            frame_count = 0;
            shutdown_requested = false;

            wz::engine::run(&per_frame_callback, this);

            if (on_end)
            {
                on_end(wz::engine::context(), last_frame);
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

    h.per_frame = [&](engine::Context &ctx, engine::FrameContext &fctx)
    {
        callback_seen = true;
    };

    h.run();

    // ✔ engine truth: harness is now authoritative for test logic
    EXPECT_EQ(h.frame_count, 10);
    EXPECT_TRUE(callback_seen);
}

TEST(EngineSmokeTest, RunsForNFrames2)
{
    EngineTest::EngineTestHarness h;

    h.max_frames = 10;

    bool callback_seen = false;
    uint64_t last_frame = 0;

    h.per_frame = [&](engine::Context &, engine::FrameContext &fctx)
    {
        callback_seen = true;
        EXPECT_GE(fctx.frame.index, last_frame);
        last_frame = fctx.frame.index;
    };

    h.run();

    EXPECT_EQ(h.frame_count, 10);
    EXPECT_TRUE(callback_seen);
}