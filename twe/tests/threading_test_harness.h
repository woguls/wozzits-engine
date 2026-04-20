#pragma once

#include <vector>
#include <thread>
#include <functional>

class ThreadTestHarness
{
public:
    template <typename Fn>
    void spawn(size_t count, Fn fn)
    {
        for (size_t i = 0; i < count; ++i)
        {
            threads.emplace_back(fn);
        }
    }

    void join()
    {
        for (auto &t : threads)
        {
            if (t.joinable())
                t.join();
        }

        threads.clear();
    }

private:
    std::vector<std::thread> threads;
};