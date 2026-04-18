#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <algorithm>

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