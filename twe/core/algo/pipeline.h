#pragma once
#include <cstdint>
#include <utility>

namespace wz::core::algo::pipeline
{
    template<typename F>
    struct map_t
    {
        F fn;

        template<typename In, typename Out>
        void operator()(const In& in, Out& out) const
        {
            for (auto& v : in)
                out.push(fn(v));
        }
    };

    template<typename F>
    map_t<F> map(F&& f)
    {
        return { std::forward<F>(f) };
    }

    template<typename P>
    struct filter_t
    {
        P pred;

        template<typename In, typename Out>
        void operator()(const In& in, Out& out) const
        {
            for (auto& v : in)
                if (pred(v))
                    out.push(v);
        }
    };

    template<typename P>
    filter_t<P> filter(P&& p)
    {
        return { std::forward<P>(p) };
    }
}

