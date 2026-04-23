#include <wozzits/event.h>

namespace wz::event
{
    wz::core::MPSCRingBuffer<Event, MAX_EVENTS> event_queue;
}