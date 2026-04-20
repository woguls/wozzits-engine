#pragma once

#include <functional>

namespace WZ
{
    struct IAsyncExecutor
    {
        virtual ~IAsyncExecutor() = default;

        virtual void post(std::function<void()> job) = 0;
    };

    void set_async_executor(IAsyncExecutor *executor);
    IAsyncExecutor *get_async_executor();
}