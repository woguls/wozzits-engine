#include <wozzits/time.h>
#include <thread>
#include <cassert>

int main()
{
    auto t1 = WZ::Time::Clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto t2 = WZ::Time::Clock::now();

    assert(t2 > t1);
}