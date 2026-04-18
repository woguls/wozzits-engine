#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <atomic>

#include "MPSC-queue.h"

using namespace WZ;

namespace
{
    MPSCQueue<int> g_queue;
    std::mutex g_output_mutex;
    std::vector<int> g_output;

    void clear_output()
    {
        std::lock_guard<std::mutex> lock(g_output_mutex);
        g_output.clear();
    }

    void drain_queue()
    {
        int value;
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
                }
            });
    }

    for (auto& th : threads)
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
                }
            });
    }

    for (auto& t : pool)
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
    int v = -1;
    EXPECT_FALSE(g_queue.try_pop(v));
}

TEST_F(MPSCQueueTest, ConcurrentPushPop)
{
    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 1000;
    const int total_items = num_producers * items_per_producer;

    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    std::atomic<bool> producers_done{false};

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int i = 0; i < num_producers; ++i)
    {
        producers.emplace_back([i, items_per_producer]()
        {
            for (int j = 0; j < items_per_producer; ++j)
            {
                g_queue.push(i * items_per_producer + j);
                push_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (int i = 0; i < num_consumers; ++i)
    {
        consumers.emplace_back([i]()
        {
            int value;
            while (true)
            {
                if (g_queue.try_pop(value))
                {
                    std::lock_guard<std::mutex> lock(g_output_mutex);
                    g_output.push_back(value);
                    pop_count.fetch_add(1, std::memory_order_relaxed);
                }
                else if (producers_done.load(std::memory_order_acquire))
                {
                    std::this_thread::yield();
                    if (g_queue.empty())
                        break;
                }
            }
        });
    }

    for (auto& t : producers) t.join();
    producers_done.store(true, std::memory_order_release);

    for (auto& t : consumers) t.join();

    EXPECT_EQ(push_count.load(), total_items);
    EXPECT_EQ(pop_count.load(), total_items);

    auto out = get_output();
    EXPECT_EQ(out.size(), total_items);

    std::unordered_set<int> seen;
    for (int v : out)
    {
        EXPECT_TRUE(seen.insert(v).second) << "Duplicate: " << v;
    }
}
