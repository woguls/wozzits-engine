#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <atomic>
#include <random>
#include <chrono>

#include "../src/core/MPSC-queue.h"

using namespace WZ::core;

namespace
{
    MPSCQueue<uint64_t> g_queue;
    std::mutex g_output_mutex;
    std::vector<int> g_output;

    void clear_output()
    {
        std::lock_guard<std::mutex> lock(g_output_mutex);
        g_output.clear();
    }

    void drain_queue()
    {
        uint64_t value;
        while (g_queue.try_pop(value))
        {
            std::lock_guard<std::mutex> lock(g_output_mutex);
            g_output.push_back(value);
        }
    }

    std::vector<int> get_output()
    {
        std::lock_guard<std::mutex> lock(g_output_mutex);
        return g_output;
    }

    class MPSCQueueTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            clear_output();
        }

        void TearDown() override
        {
            drain_queue();
        }
    };
}

TEST_F(MPSCQueueTest, SingleThreadPushPop)
{
    g_queue.push(1);
    g_queue.push(2);
    g_queue.push(3);

    drain_queue();

    auto out = get_output();
    EXPECT_EQ(out.size(), 3);
    EXPECT_EQ(out[0], 1);
    EXPECT_EQ(out[1], 2);
    EXPECT_EQ(out[2], 3);
}

TEST_F(MPSCQueueTest, MultiProducerCorrectness)
{
    const int num_threads = 8;
    const int items_per_thread = 1000;

    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([t, items_per_thread]()
                             {
                for (int i = 0; i < items_per_thread; ++i)
                {
                    g_queue.push(t * 100000 + i);
                } });
    }

    for (auto &th : threads)
        th.join();

    drain_queue();

    auto out = get_output();
    EXPECT_EQ(out.size(), num_threads * items_per_thread);

    std::unordered_set<int> seen;
    for (int v : out)
    {
        EXPECT_TRUE(seen.insert(v).second) << "Duplicate: " << v;
    }
}

TEST_F(MPSCQueueTest, FIFOApproximationSingleConsumer)
{
    const int n = 10000;

    for (int i = 0; i < n; ++i)
        g_queue.push(i);

    drain_queue();

    auto out = get_output();
    EXPECT_EQ(out.size(), n);

    for (int i = 1; i < n; ++i)
    {
        EXPECT_NE(out[i], out[i - 1]);
    }
}

TEST_F(MPSCQueueTest, HighContentionStress)
{
    const int threads = 16;
    const int per_thread = 5000;

    std::vector<std::thread> pool;

    for (int t = 0; t < threads; ++t)
    {
        pool.emplace_back([t, per_thread]()
                          {
                for (int i = 0; i < per_thread; ++i)
                {
                    g_queue.push(t * 100000 + i);
                } });
    }

    for (auto &t : pool)
        t.join();

    drain_queue();

    auto out = get_output();
    EXPECT_EQ(out.size(), threads * per_thread);

    std::unordered_set<int> seen;
    for (auto v : out)
        EXPECT_TRUE(seen.insert(v).second);
}

TEST_F(MPSCQueueTest, EmptyQueueBehavior)
{
    uint64_t v = -1;
    EXPECT_FALSE(g_queue.try_pop(v));
}

TEST_F(MPSCQueueTest, ConcurrentProducerConsumer)
{
    const int producers = 2;
    const int per_thread = 5;

    std::atomic<bool> start{false};
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    std::vector<std::thread> threads;

    // Consumer
    std::thread consumer([&]()
                         {
            while (!start.load()) {}

            uint64_t value;
            while (consumed.load() < producers * per_thread)
            {
                if (g_queue.try_pop(value))
                {
                    consumed++;
                }
            } });

    // Producers
    for (int t = 0; t < producers; ++t)
    {
        threads.emplace_back([&, t]()
                             {
                while (!start.load()) {}

                for (int i = 0; i < per_thread; ++i)
                {
                    g_queue.push(t * 100000 + i);
                    produced++;
                } });
    }

    start = true;

    for (auto &th : threads)
        th.join();

    consumer.join();

    EXPECT_EQ(produced.load(), producers * per_thread);
    EXPECT_EQ(consumed.load(), producers * per_thread);
}

TEST_F(MPSCQueueTest, Stress)
{
    const int producers = 8;
    const int per_thread = 200000;
    const int total = producers * per_thread;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    std::vector<std::thread> threads;

    // consumer thread
    std::vector<int> results;
    results.reserve(total);

    std::thread consumer([&]()
                         {
        uint64_t value;

        while (consumed.load(std::memory_order_relaxed) < total)
        {
            if (g_queue.try_pop(value))
            {
                results.push_back(value);
                consumed.fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                std::this_thread::yield();
            }
        } });

    // producer threads
    for (int t = 0; t < producers; ++t)
    {
        threads.emplace_back([&, t]()
                             {
            for (int i = 0; i < per_thread; ++i)
            {
                int value = t * per_thread + i;

                g_queue.push(value);

                produced.fetch_add(1, std::memory_order_relaxed);

                // optional: introduce scheduling noise
                if ((i & 255) == 0)
                    std::this_thread::yield();
            } });
    }

    for (auto &th : threads)
        th.join();

    consumer.join();

    EXPECT_EQ(produced.load(), total);
    EXPECT_EQ(consumed.load(), total);

    // verify uniqueness
    std::unordered_set<int> seen;
    seen.reserve(total);

    for (int v : results)
    {
        EXPECT_TRUE(seen.insert(v).second) << "Duplicate: " << v;
    }

    EXPECT_EQ(seen.size(), total);
}

TEST_F(MPSCQueueTest, ChaosStress)
{
    const int producers = 128;
    const int duration_seconds = 180;

    std::atomic<bool> running{true};
    std::atomic<uint64_t> produced{0};
    std::atomic<uint64_t> consumed{0};

    std::vector<std::thread> threads;

    // Per-producer sequence tracking (must persist across entire run)
    std::vector<int64_t> last_seen(producers, -1);

    std::atomic<int> violations{0};

    // Consumer
    std::thread consumer([&]()
                         {
        uint64_t value;

        while (running.load(std::memory_order_acquire))
        {
            if (g_queue.try_pop(value))
            {
                consumed.fetch_add(1, std::memory_order_relaxed);

                uint64_t producer = value >> 48;
                uint64_t seq = value & 0x0000FFFFFFFFFFFFULL;

                if (last_seen[producer] != -1)
                {
                    if (seq != static_cast<uint64_t>(last_seen[producer] + 1))
                        violations.fetch_add(1, std::memory_order_relaxed);
                }

                last_seen[producer] = seq;
            }
            else
            {
                std::this_thread::yield();
            }
        }

        // Drain remaining items
        while (g_queue.try_pop(value))
        {
            consumed.fetch_add(1, std::memory_order_relaxed);

            uint64_t producer = value >> 48;
            uint64_t seq = value & 0x0000FFFFFFFFFFFFULL;

            if (last_seen[producer] != -1)
            {
                if (seq != static_cast<uint64_t>(last_seen[producer] + 1))
                    violations.fetch_add(1, std::memory_order_relaxed);
            }

            last_seen[producer] = seq;
        } });

    // Producers
    for (int t = 0; t < producers; ++t)
    {
        threads.emplace_back([&, t]()
                             {
            std::mt19937 rng(t + 1234);
            std::uniform_int_distribution<int> spin(0, 50);

            uint64_t local = 0;

            while (running.load(std::memory_order_acquire))
            {
                int burst = spin(rng);

                for (int i = 0; i < burst; ++i)
                {
                    uint64_t v = ((uint64_t)t << 48) | local++;

                    g_queue.push(v);
                    produced.fetch_add(1, std::memory_order_relaxed);
                }

                if (spin(rng) < 5)
                    std::this_thread::yield();
            } });
    }

    std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));

    running.store(false, std::memory_order_release);

    for (auto &th : threads)
        th.join();

    consumer.join();

    uint64_t p = produced.load();
    uint64_t c = consumed.load();

    EXPECT_EQ(p, c);

    EXPECT_EQ(violations.load(), 0);
}