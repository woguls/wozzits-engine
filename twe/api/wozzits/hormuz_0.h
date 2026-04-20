#pragma once

#include <cstdint>
#include <array>
#include <wozzits/window_types.h>

namespace WZ::hormuz
{

    using Event = WZ::window::WindowEvent;
    using EventType = WZ::window::WindowEventType;

    using EventCallback = void (*)(const Event &, void *user);

    struct Listener
    {
        EventCallback callback;
        void *user;
    };

    constexpr uint32_t MAX_EVENT_TYPES = 64;
    constexpr uint32_t MAX_LISTENERS_PER_TYPE = 32;

    class EventBus
    {
    public:
        void subscribe(EventType type, EventCallback cb, void *user);

        void publish(const Event &e);

        void reset();

    private:
        struct ListenerList
        {
            uint32_t count = 0;
            Listener listeners[MAX_LISTENERS_PER_TYPE];
        };

        std::array<ListenerList, MAX_EVENT_TYPES> m_lists;
    };

    EventBus &instance();

}