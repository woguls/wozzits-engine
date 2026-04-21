#include <cstdint>
#include <wozzits/event.h>

namespace wz::input
{
    struct KeyboardState
    {
        bool down[256]{};
        bool pressed[256]{};
        bool released[256]{};
    };

    struct MouseState
    {
        int x = 0;
        int y = 0;
        int dx = 0;
        int dy = 0;

        bool down[3]{};
        bool pressed[3]{};
        bool released[3]{};
    };

    struct WindowState
    {
        bool focused = true;
        int width = 0;
        int height = 0;
    };

    struct ControllerState
    {
        bool connected = false;
        float axes[8]{};
        bool buttons[16]{};
    };

    struct InputState
    {
        KeyboardState keyboard;
        MouseState mouse;
        WindowState window;
        ControllerState controller;
    };

    void build_input(InputState &input,
                     const wz::event::Event *events,
                     size_t count,
                     wz::time::Tick frame_start,
                     wz::time::Tick frame_end);
}