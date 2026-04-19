#include "../window.h"
#include "../../src/platform/win32.h"

namespace WZ::window
{
    WindowHandle create_window(const WindowDesc &desc)
    {
        return WZ::platform::win32::w32_create_window(desc);
    }

    void destroy_window(WindowHandle window)
    {
        WZ::platform::win32::w32_destroy_window(window);
    }

    bool window_should_close(WindowHandle handle)
    {
        return WZ::platform::win32::w32_window_should_close(handle);
    }

    void pump_messages()
    {
        WZ::platform::win32::w32_pump_messages();
    }

    bool poll_event(WindowHandle window, WindowEvent &out_event)
    {
        return WZ::platform::win32::w32_poll_event(window, out_event);
    }

}