#include <wozzits/event.h>

namespace wz::event
{
    struct EventQueueSink : EventSink
    {
        bool try_push(const Event &e) override
        {
            return event_queue.try_push(e);
        }
    };
}