#pragma once

#include "window_types.h"

namespace WZ::window
{

    WindowHandle create_window(const WindowDesc &desc);
    void destroy_window(WindowHandle window);
    bool window_should_close(WindowHandle window);

    bool poll_event(WindowHandle window, WindowEvent &out_event);

    void pump_messages();
}