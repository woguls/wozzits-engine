#include <wozzits/time.h>
#include <thread>
#include <cassert>

int main()
{
    auto t1 = wz::time::Clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto t2 = wz::time::Clock::now();

    assert(t2 > t1);
}