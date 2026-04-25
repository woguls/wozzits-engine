#pragma once
#include <cstdint>

namespace wz::core::containers
{
    /**
     * @brief Simple contiguous buffer view used throughout RenderIR and elsewhere.
     *
     * This does not own memory. It is assumed to point into arena-allocated
     * or externally managed storage.
     *
     * @tparam T Element type stored in the buffer.
     * 
     * TODO: “core::containers::Span + Buffer interaction model” for zero-copy frame building
     */

        template<typename T>
        struct Buffer
        {
            T* data = nullptr;
            uint32_t count = 0;
            uint32_t capacity = 0;

            void reset()
            {
                count = 0;
            }

            bool push(const T& value)
            {
                if (count >= capacity)
                    return false;

                data[count++] = value;
                return true;
            }

            bool has_space(uint32_t n = 1) const
            {
                return count + n <= capacity;
            }

            static Buffer<T> wrap(T* data, uint32_t capacity)
            {
                Buffer<T> b;
                b.data = data;
                b.count = 0;
                b.capacity = capacity;
                return b;
            }

        };
    
}