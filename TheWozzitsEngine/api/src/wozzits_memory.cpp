#include "../wozzits_memory.h"
#include <cstdlib>

namespace WZ::Memory
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