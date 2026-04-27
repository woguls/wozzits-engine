#pragma once
#include <cstdint>
#include <utility>

/*
ALGO EXECUTION MODEL:

- Buffer is a (pointer, count) range
- algo functions MAY NOT:
    - store pointers/references beyond call scope
    - modify input buffer state
    - assume ownership or lifetime extension
    - allocate memory
- all operations are deterministic given identical input
*/

namespace wz::core::algo
{
    template<typename BufferT, typename F>
    void for_each(BufferT& b, F&& fn)
    {
        auto* data = b.data();
        auto n = b.count();

        for (uint32_t i = 0; i < n; ++i)
        {
            fn(data[i]);
        }
    }

    template<typename BufferT, typename F, typename OutBufferT>
    void transform(const BufferT& in, OutBufferT& out, F&& fn)
    {
        auto* data = in.data();
        auto n = in.count();

        for (uint32_t i = 0; i < n; ++i)
        {
            out.push(fn(data[i]));
        }
    }

    template<typename BufferT, typename T, typename F>
    T reduce(const BufferT& b, T init, F&& fn)
    {
        auto* data = b.data();
        auto n = b.count();

        for (uint32_t i = 0; i < n; ++i)
        {
            init = fn(init, data[i]);
        }

        return init;
    }
}