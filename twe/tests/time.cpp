#include <wozzits/w_time.h>
#include <thread>
#include <cassert>

int main()
{
    auto t1 = wz::time::TimeSource::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto t2 = wz::time::TimeSource::now();

    assert(t2 > t1);
}