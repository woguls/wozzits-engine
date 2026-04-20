#include <wozzits/memory.h>

#include <cstdlib>

namespace wz::memory
{
    void *alloc(size_t size)
    {
        return std::malloc(size);
    }

    void free(void *ptr)
    {
        std::free(ptr);
    }

}