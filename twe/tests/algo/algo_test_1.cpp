#include <gtest/gtest.h>
#include <containers/buffer.h>
#include <algo/algo.h>

using namespace wz::core;

TEST(Algo, ForEachVisitsAllElements)
{
    int storage[3] = { 1, 2, 3 };
    auto b = containers::Buffer<int>::wrap(storage, 3);

    // wrap() does NOT imply filled data
    EXPECT_EQ(b.count(), 0);
    EXPECT_EQ(b.capacity(), 3);

    // simulate “valid filled buffer”
    b.push(1);
    b.push(2);
    b.push(3);

    int sum = 0;

    algo::for_each(b, [&](int v)
        {
            sum += v;
        });

    EXPECT_EQ(b.count(), 3);
    EXPECT_EQ(sum, 6);
}

TEST(Algo, ForEachDoesNotModifyBuffer)
{
    int storage[3] = { 1, 2, 3 };
    auto b = containers::Buffer<int>::wrap_existing(storage, 3, 3);

    algo::for_each(b, [](int& v)
        {
            // intentionally empty
        });

    EXPECT_EQ(storage[0], 1);
    EXPECT_EQ(storage[1], 2);
    EXPECT_EQ(storage[2], 3);
}

TEST(Algo, TransformProducesCorrectOutput)
{
    int in_storage[3] = { 1, 2, 3 };
    int out_storage[3] = { 0, 0, 0 };

    auto in = containers::Buffer<int>::wrap_existing(in_storage, 3, 3);
    auto out = containers::Buffer<int>::wrap(out_storage, 3);

    algo::transform(in, out, [](int v)
        {
            return v * 2;
        });

    EXPECT_EQ(out_storage[0], 2);
    EXPECT_EQ(out_storage[1], 4);
    EXPECT_EQ(out_storage[2], 6);
}

TEST(Algo, TransformRespectsOutputCapacity)
{
    int in_storage[3] = { 1, 2, 3 };
    int out_storage[2] = { 0, 0 };

    auto in = containers::Buffer<int>::wrap_existing(in_storage, 3, 3);
    auto out = containers::Buffer<int>::wrap(out_storage, 2);

    algo::transform(in, out, [](int v)
        {
            return v;
        });

    EXPECT_EQ(out.count(), 2);
    EXPECT_EQ(out_storage[0], 1);
    EXPECT_EQ(out_storage[1], 2);
}

TEST(Algo, ReduceIsDeterministic)
{
    int storage[4] = { 1, 2, 3, 4 };
    auto b = containers::Buffer<int>::wrap_existing(storage, 4, 4);

    int result = algo::reduce(b, 0, [](int acc, int v)
        {
            return acc + v;
        });

    EXPECT_EQ(result, 10);
}

TEST(Algo, EmptyBufferIsSafe)
{
    int storage[1];
    auto b = containers::Buffer<int>::wrap(storage, 0);

    int count = 0;

    algo::for_each(b, [&](int)
        {
            count++;
        });

    EXPECT_EQ(count, 0);
}

using namespace wz::core::containers;

TEST(AlgoSpec, ForEachDoesNotChangeBufferSize)
{
    int storage[3] = { 1,2,3 };
    auto b = Buffer<int>::wrap_existing(storage, 3, 3);

    auto before = b.count();

    algo::for_each(b, [](int&) {});

    EXPECT_EQ(b.count(), before);
}

TEST(AlgoSpec, TransformDoesNotMutateInput)
{
    int in_storage[3] = { 1,2,3 };
    int out_storage[3] = {};

    auto in = Buffer<int>::wrap_existing(in_storage, 3, 3);
    auto out = Buffer<int>::wrap(out_storage, 3);

    algo::transform(in, out, [](int v) { return v * 2; });

    EXPECT_EQ(in_storage[0], 1);
    EXPECT_EQ(in_storage[1], 2);
    EXPECT_EQ(in_storage[2], 3);
}

TEST(AlgoSpec, ReduceDoesNotMutateBuffer)
{
    int storage[3] = { 1,2,3 };
    auto b = Buffer<int>::wrap_existing(storage, 3, 3);

    int result = algo::reduce(b, 0, [](int acc, int v)
        {
            return acc + v;
        });

    EXPECT_EQ(storage[0], 1);
    EXPECT_EQ(storage[1], 2);
    EXPECT_EQ(storage[2], 3);
}

TEST(AlgoSpec, ForEachIsDeterministicOrder)
{
    int storage[4] = { 1,2,3,4 };
    auto b = Buffer<int>::wrap_existing(storage, 4, 4);

    std::vector<int> seen;

    algo::for_each(b, [&](int v)
        {
            seen.push_back(v);
        });

    EXPECT_EQ(seen, (std::vector<int>{1, 2, 3, 4}));
}

TEST(AlgoSpec, ReduceIsDeterministic)
{
    int storage[4] = { 1,2,3,4 };
    auto b = Buffer<int>::wrap_existing(storage, 4, 4);

    int result = algo::reduce(b, 0, [](int acc, int v)
        {
            return acc + v;
        });

    EXPECT_EQ(result, 10);
}

TEST(AlgoSpec, TransformRespectsOutputCapacity)
{
    int in_storage[3] = { 1,2,3 };
    int out_storage[2] = {};

    auto in = Buffer<int>::wrap_existing(in_storage, 3, 3);
    auto out = Buffer<int>::wrap(out_storage, 2);

    algo::transform(in, out, [](int v) { return v; });

    EXPECT_EQ(out.count(), 2);
    EXPECT_EQ(out_storage[0], 1);
    EXPECT_EQ(out_storage[1], 2);
}

TEST(AlgoSpec, TransformDoesNotCrashOnOverflow)
{
    int in_storage[3] = { 1,2,3 };
    int out_storage[1] = {};

    auto in = Buffer<int>::wrap_existing(in_storage, 3, 3);
    auto out = Buffer<int>::wrap(out_storage, 1);

    algo::transform(in, out, [](int v) { return v; });

    EXPECT_GE(out.overflow_count(), 0);
}

TEST(AlgoSpec, EmptyBufferSafe)
{
    int storage[1];
    auto b = Buffer<int>::wrap(storage, 0);

    int count = 0;

    algo::for_each(b, [&](int) { count++; });

    EXPECT_EQ(count, 0);
}

TEST(AlgoSpec, ZeroCapacityNoCrash)
{
    auto b = Buffer<int>::wrap(nullptr, 0);

    int result = algo::reduce(b, 123, [](int acc, int v)
        {
            return acc + v;
        });

    EXPECT_EQ(result, 123);
}

TEST(AlgoSpec, DoesNotAccessBeyondCount)
{
    int storage[5] = { 1,2,3,999,999 };
    auto b = Buffer<int>::wrap_existing(storage, 3, 5);

    int sum = 0;

    algo::for_each(b, [&](int v)
        {
            sum += v;
        });

    EXPECT_EQ(sum, 6); // must ignore tail garbage
}


