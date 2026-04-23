#include <wozzits/input.h>

namespace wz::input
{
    void build_input(InputState &input,
                     const wz::event::Event *events,
                     size_t count,
                     wz::time::Frame frame)
    {

        //  phase 1 Reset input state
        input.keyboard = {};
        input.mouse = {};
        input.window = {};
        input.controller = {};

        // phase 2 Iterate events
        for (size_t i = 0; i < count; ++i)
        {
            const auto &e = events[i];

            // phase 3 Reduce evets -> state
            switch (e.type)
            {
            case wz::event::Event::Type::KeyPressDown:
                input.keyboard.down[e.key_code] = true;
                input.keyboard.pressed[e.key_code] = true;
                break;

            case wz::event::Event::Type::KeyPressUp:
                input.keyboard.down[e.key_code] = false;
                input.keyboard.released[e.key_code] = true;
                break;

            case wz::event::Event::Type::MouseMove:
                input.mouse.dx += e.mouse_dx;
                input.mouse.dy += e.mouse_dy;
                input.mouse.x = e.mouse_x;
                input.mouse.y = e.mouse_y;
                break;

            case wz::event::Event::Type::MouseButton:
                input.mouse.down[e.button] = e.pressed;
                input.mouse.pressed[e.button] |= e.pressed;
                input.mouse.released[e.button] |= !e.pressed;
                break;

            case wz::event::Event::Type::Resized:
                input.window.width = e.width;
                input.window.height = e.height;
                break;

            case wz::event::Event::Type::FocusGained:
                input.window.focused = true;
                break;

            case wz::event::Event::Type::FocusLost:
                input.window.focused = false;
                break;

            case wz::event::Event::Type::ControllerAxis:
                input.controller.axes[e.axis] = e.value;
                break;

            case wz::event::Event::Type::ControllerButton:
                input.controller.buttons[e.button] = e.pressed;
                break;
            }

            // leave dx/dy as-is for simulation, or explicitly define policy
            // input.mouse.dx = 0; // optional depending on design
        }
    }
}
