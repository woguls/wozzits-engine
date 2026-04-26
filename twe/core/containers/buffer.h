#pragma once
#include <cstdint>
#include <cassert>
#include <utility>

namespace wz::core::containers
{
    template<typename T>
    struct Buffer
    {
        T* data = nullptr;
        uint32_t count = 0;
        uint32_t capacity = 0;
        uint32_t overflow_count = 0;

        void reset()
        {
            count = 0;
        }

        bool push(const T& value)
        {
            if (count >= capacity)
            {
                overflow_count++;
                return false;
            }
                

            assert(data != nullptr || capacity == 0);

            data[count++] = value; // copy
            return true;
        }

        bool push(T&& value)
        {
            if (count >= capacity)
                return false;

            assert(data != nullptr || capacity == 0);

            data[count++] = std::move(value); // move
            return true;
        }

        bool try_push(const T& value)
        {
            return push(value);
        }

        bool has_space(uint32_t n = 1) const
        {
            assert(count <= capacity);
            return n <= (capacity - count); // avoids overflow
        }

        uint32_t remaining() const
        {
            return capacity - count;
        }

        static Buffer<T> wrap(T* data, uint32_t capacity)
        {
            assert(data != nullptr || capacity == 0);

            return Buffer<T>{
                .data = data,
                    .count = 0,
                    .capacity = capacity
            };
        }

        static Buffer<T> wrap_existing(T* data, uint32_t count, uint32_t capacity)
        {
            assert(count <= capacity);

            return Buffer<T>{
                .data = data,
                    .count = count,
                    .capacity = capacity
            };
        }
    };
}