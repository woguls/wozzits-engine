#pragma once

#include <cstdint>
#include <array>
#include <wozzits/window_types.h>

namespace wz::hormuz
{
    /**
     * @brief Alias for platform window events.
     *
     * Hormuz operates on platform-level events only.
     * It is NOT part of the simulation/event_queue/input pipeline.
     */
    using Event = wz::window::WindowEvent;

    /**
     * @brief Type identifier for window events.
     */
    using EventType = wz::window::WindowEventType;

    /**
     * @brief Callback invoked when a subscribed event is published.
     *
     * @param event The event being dispatched.
     * @param user  User-defined context pointer provided at subscription time.
     */
    using EventCallback = void (*)(const Event &event, void *user);

    /**
     * @brief Listener binding for an event subscription.
     *
     * Stores a callback and an opaque user pointer.
     */
    struct Listener
    {
        EventCallback callback = nullptr;
        void *user = nullptr;
    };

    /**
     * @brief Maximum number of event types supported by the bus.
     */
    constexpr uint32_t MAX_EVENT_TYPES = 64;

    /**
     * @brief Maximum number of listeners per event type.
     */
    constexpr uint32_t MAX_LISTENERS_PER_TYPE = 32;

    /**
     * @brief Lightweight event dispatch system for platform/window events.
     *
     * @warning Hormuz is NOT part of the engine simulation pipeline.
     *
     * ### Architectural role
     * Hormuz exists purely as a diagnostic and platform event distribution layer.
     *
     * It is used for:
     * - Window system notifications
     * - Platform-level event fan-out
     * - Debug tooling / editor integration (future use)
     *
     * It is NOT used for:
     * - Input state building
     * - Frame simulation
     * - Gameplay event processing
     *
     * ### Design constraints
     * - Must not influence frame timing or simulation correctness
     * - May be lossy under load (best-effort delivery)
     * - Must not be required for deterministic replay
     * - Callbacks must not assume ordering guarantees beyond registration order
     */
    class EventBus
    {
    public:
        /**
         * @brief Subscribe to a specific event type.
         *
         * @param type Event type to listen for.
         * @param cb   Callback function invoked when event is published.
         * @param user Opaque user pointer passed to callback.
         */
        void subscribe(EventType type, EventCallback cb, void *user);

        /**
         * @brief Publish an event to all registered listeners.
         *
         * @warning This is a diagnostic dispatch mechanism only.
         * It must NOT be used for simulation-critical logic.
         *
         * Callbacks are executed synchronously in publish order.
         */
        void publish(const Event &e);

        /**
         * @brief Removes all listeners from the bus.
         *
         * Intended for shutdown, reset, or test isolation.
         */
        void reset();

    private:
        struct ListenerList
        {
            uint32_t count = 0;
            Listener listeners[MAX_LISTENERS_PER_TYPE];
        };

        std::array<ListenerList, MAX_EVENT_TYPES> m_lists;
    };

    /**
     * @brief Global singleton instance of the Hormuz event bus.
     *
     * @warning This is a diagnostic system. Do not route gameplay or input
     * through this instance.
     */
    EventBus &instance();

} // namespace wz::hormuz