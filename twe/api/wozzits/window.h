#pragma once

#include "window_types.h"

namespace wz::window
{

    WindowHandle create_window(const WindowDesc &desc);
    void destroy_window(WindowHandle window);

    bool should_close(WindowHandle window);

    void pump_messages(); // OS integration (Win32 message pump)

    bool poll_event(WindowHandle window, WindowEvent &out);
}