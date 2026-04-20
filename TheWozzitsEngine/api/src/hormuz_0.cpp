#include "../hormuz_0.h"

namespace WZ::hormuz
{

    static EventBus g_bus;

    EventBus &instance()
    {
        return g_bus;
    }

    void EventBus::reset()
    {
        for (auto &list : m_lists)
        {
            list.count = 0;
        }
    }

    void EventBus::subscribe(EventType type, EventCallback cb, void *user)
    {
        uint32_t index = static_cast<uint32_t>(type);

        auto &list = m_lists[index];

        if (list.count >= MAX_LISTENERS_PER_TYPE)
            return; // drop if full

        list.listeners[list.count++] = {cb, user};
    }

    void EventBus::publish(const Event &e)
    {
        uint32_t index = static_cast<uint32_t>(e.type);

        auto &list = m_lists[index];

        for (uint32_t i = 0; i < list.count; i++)
        {
            auto &listener = list.listeners[i];
            listener.callback(e, listener.user);
        }
    }

}