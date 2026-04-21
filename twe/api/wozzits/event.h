#include <cstdint>
#include <wozzits/w_time.h>
#include <wozzits/mpsc_queue.h>

namespace wz::event
{
    static constexpr size_t MAX_EVENTS = 512; // unused ?

    struct Event
    {
        enum class Category
        {
            Input,
            Window,
            Controller,
            Mouse,
            VR
        };

        enum class Source
        {
            Platform,
            Engine,
            Game
        };

        enum class Type : uint16_t
        {
            None,

            // Keyboard
            KeyPressDown,
            KeyPressUp,
            KeyHoldBegin,
            KeyHoldEnd,

            // Mouse
            MouseMove,
            MouseButton,

            // Window
            Resized,
            Closed,
            FocusGained,
            FocusLost,

            // Controller
            ControllerButton,
            ControllerAxis,

            // VR
            VREvent
        };

        wz::time::Tick timestamp;
        Source source;
        Type type;
    };

    wz::core::MPSCQueue<wz::event::Event> event_queue;

}