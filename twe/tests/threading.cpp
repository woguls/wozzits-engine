#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <atomic>
#include <random>
#include <chrono>
#include <queue>

#include <wozzits/threading.h>
#include "threading_test_harness.h"

using namespace wz;

namespace
{

    class StressTest_A
        : public ::testing::TestWithParam<int>
    {
    };

    class StressTest_B
        : public ::testing::TestWithParam<int>
    {
    };

    class StressTest_C
        : public ::testing::TestWithParam<int>
    {
    };
} // namespace

TEST_P(StressTest_A, BasicThreadExecution)
{
    ThreadTestHarness harness;

    std::atomic<int> counter = 0;

    harness.spawn(4, [&]()
                  { counter.fetch_add(1, std::memory_order_relaxed); });

    harness.join();

    EXPECT_EQ(counter.load(), 4);
}

TEST_P(StressTest_A, AtomicContention)
{
    ThreadTestHarness harness;

    std::atomic<int> counter = 0;

    harness.spawn(8, [&]()
                  {
        for (int i = 0; i < 10000; ++i)
        {
            counter.fetch_add(1, std::memory_order_relaxed);
        } });

    harness.join();

    EXPECT_EQ(counter.load(), 80000);
}

TEST_P(StressTest_A, UniqueThreadIDs)
{
    ThreadTestHarness harness;

    std::mutex lock;
    std::unordered_set<std::thread::id> ids;

    harness.spawn(8, [&]()
                  {
        std::lock_guard<std::mutex> guard(lock);
        ids.insert(std::this_thread::get_id()); });

    harness.join();

    EXPECT_EQ(ids.size(), 8);
}

TEST_P(StressTest_A, HighContentionIncrement)
{
    ThreadTestHarness harness;

    const int threads = 8;
    const int iterations = 10000;

    std::atomic<int> counter = 0;

    harness.spawn(threads, [&]()
                  {
        for (int i = 0; i < iterations; i++)
        {
            counter.fetch_add(1, std::memory_order_relaxed);
        } });

    harness.join();

    EXPECT_EQ(counter.load(), threads * iterations);
}

TEST_P(StressTest_A, BarrierStart)
{
    ThreadTestHarness harness;

    std::atomic<int> ready = 0;
    std::atomic<int> started = 0;

    const int threads = 8;

    harness.spawn(threads, [&]()
                  {
        ready.fetch_add(1);

        while (ready.load() < threads)
        {
            std::this_thread::yield();
        }

        started.fetch_add(1); });

    harness.join();

    EXPECT_EQ(started.load(), threads);
}

TEST_P(StressTest_A, RapidCreateDestroy)
{
    const int cycles = 100;

    for (int i = 0; i < cycles; i++)
    {
        ThreadTestHarness harness;

        harness.spawn(4, []() {});

        harness.join();
    }

    SUCCEED();
}

INSTANTIATE_TEST_SUITE_P(
    StressRuns,
    StressTest_A,
    ::testing::Range(0, 50));

TEST_P(StressTest_B, RandomizedYieldStorm)
{
    ThreadTestHarness harness;

    const int threads = 8;
    const int iterations = 10000;

    std::atomic<int> counter = 0;

    harness.spawn(threads, [&]()
                  {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 10);

        for (int i = 0; i < iterations; i++)
        {
            counter.fetch_add(1, std::memory_order_relaxed);

            if (dist(rng) == 0)
                std::this_thread::yield();
        } });

    harness.join();

    EXPECT_EQ(counter.load(), threads * iterations);
}

TEST_P(StressTest_B, ThreadExplosion)
{
    ThreadTestHarness harness;

    const int threads = 64;

    std::atomic<int> counter = 0;

    harness.spawn(threads, [&]()
                  { counter.fetch_add(1); });

    harness.join();

    EXPECT_EQ(counter.load(), threads);
}

TEST_P(StressTest_B, NestedSpawn)
{
    ThreadTestHarness harness;

    std::atomic<int> counter = 0;

    harness.spawn(4, [&]()
                  {
        counter.fetch_add(1);

        std::thread inner([&]()
        {
            counter.fetch_add(1);
        });

        inner.join(); });

    harness.join();

    EXPECT_EQ(counter.load(), 8);
}

INSTANTIATE_TEST_SUITE_P(
    StressRuns,
    StressTest_B,
    ::testing::Range(0, 50));

TEST_P(StressTest_C, MutexContention)
{
    ThreadTestHarness harness;

    std::mutex m;
    int counter = 0;

    const int threads = 8;
    const int iterations = 5000;

    harness.spawn(threads, [&]()
                  {
        for (int i = 0; i < iterations; i++)
        {
            std::lock_guard<std::mutex> lock(m);
            counter++;
        } });

    harness.join();

    EXPECT_EQ(counter, threads * iterations);
}

TEST_P(StressTest_C, SharedVectorPush)
{
    ThreadTestHarness harness;

    std::mutex m;
    std::vector<int> values;

    const int threads = 8;
    const int iterations = 1000;

    harness.spawn(threads, [&]()
                  {
        for (int i = 0; i < iterations; i++)
        {
            std::lock_guard<std::mutex> lock(m);
            values.push_back(i);
        } });

    harness.join();

    EXPECT_EQ(values.size(), threads * iterations);
}

TEST_P(StressTest_C, RandomSleepChaos)
{
    ThreadTestHarness harness;

    std::atomic<int> counter = 0;

    const int threads = 8;
    const int iterations = 2000;

    harness.spawn(threads, [&]()
                  {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 50);

        for (int i = 0; i < iterations; i++)
        {
            counter.fetch_add(1);

            if (dist(rng) == 0)
                std::this_thread::sleep_for(std::chrono::microseconds(1));
        } });

    harness.join();

    EXPECT_EQ(counter.load(), threads * iterations);
}

TEST_P(StressTest_C, ProducerConsumerStorm)
{
    const int producers = 4;
    const int consumers = 4;
    const int tasks_per_producer = 2000;

    std::queue<int> q;
    std::mutex m;
    std::atomic<int> produced = 0;
    std::atomic<int> consumed = 0;

    ThreadTestHarness harness;

    // producers
    harness.spawn(producers, [&]()
                  {
        for (int i = 0; i < tasks_per_producer; i++)
        {
            {
                std::lock_guard<std::mutex> lock(m);
                q.push(1);
            }
            produced.fetch_add(1);
        } });

    // consumers
    harness.spawn(consumers, [&]()
                  {
        while (consumed.load() < producers * tasks_per_producer)
        {
            bool got = false;

            {
                std::lock_guard<std::mutex> lock(m);

                if (!q.empty())
                {
                    q.pop();
                    got = true;
                }
            }

            if (got)
                consumed.fetch_add(1);
            else
                std::this_thread::yield();
        } });

    harness.join();

    EXPECT_EQ(produced.load(), producers * tasks_per_producer);
    EXPECT_EQ(consumed.load(), produced.load());
}

INSTANTIATE_TEST_SUITE_P(
    StressRuns,
    StressTest_C,
    ::testing::Range(0, 10));