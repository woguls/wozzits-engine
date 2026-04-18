#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <atomic>
#include <random>
#include <chrono>

#include "../api/logger.h"
#include "thread_test_harness.h"

using namespace WZ;

namespace
{

    // Additional Test Harness stuff could go here.

} // namespace

TEST(ThreadTestHarness, NullHarnessTest)
{
    ThreadTestHarness harness;

    const int threads = 8;
    std::atomic<int> counter{0};

    harness.spawn(threads, [&](int)
                  { counter.fetch_add(1, std::memory_order_relaxed); });

    // Threads should be waiting at the start barrier
    EXPECT_EQ(counter.load(), 0);

    harness.start();
    harness.join_all();

    // After start + join, all threads must have run exactly once
    EXPECT_EQ(counter.load(), threads);
}

TEST(ThreadTestHarness, BarrierCorrectness)
{
    ThreadTestHarness harness;

    const int threads = 16;

    std::atomic<int> entered{0};
    std::atomic<int> finished{0};

    harness.spawn(threads, [&](int)
                  {
        entered.fetch_add(1, std::memory_order_relaxed);

        // simulate work
        std::this_thread::yield();

        finished.fetch_add(1, std::memory_order_relaxed); });

    // Give threads time to start and block on the barrier
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // If the harness is correct, none should have entered yet
    EXPECT_EQ(entered.load(), 0);
    EXPECT_EQ(finished.load(), 0);

    harness.start();
    harness.join_all();

    EXPECT_EQ(entered.load(), threads);
    EXPECT_EQ(finished.load(), threads);
}

TEST(LoggerTest, WorkerStartsAutomatically)
{
    WZ::Logger logger;
    logger.set_callback(LogSinkType::Buffer);
    logger.info("hello");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto buffer = logger.snapshot_memory();
    ASSERT_EQ(buffer.size(), 1);
}

TEST(LoggerTest, WorkerStopsOnDestruction)
{
    std::vector<LogEvent> snapshot;

    {
        WZ::Logger logger;
        logger.set_callback(LogSinkType::Buffer);

        logger.info("msg");

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        snapshot = logger.snapshot_memory();
    }

    ASSERT_EQ(snapshot.size(), 1);
}

TEST(LoggerTest, MultiThreadLogging)
{
    WZ::Logger logger;

    constexpr int threads = 8;
    constexpr int per_thread = 1000;

    logger.set_callback(LogSinkType::Buffer);

    ThreadTestHarness harness;

    harness.spawn(threads, [&](int)
                  {
        for (int i = 0; i < per_thread; ++i)
            logger.info("msg"); });

    harness.start();
    harness.join_all();

    logger.wait_until_idle(); // 🔥 key fix

    auto buffer = logger.snapshot_memory();

    EXPECT_EQ(buffer.size(), threads * per_thread);
}