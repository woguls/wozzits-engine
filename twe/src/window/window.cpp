#include <wozzits/window.h>
#include <win32/win32.h>

namespace wz::window
{
    WindowHandle create_window(const WindowDesc &desc)
    {
        return wz::platform::win32::w32_create_window(desc);
    }

    void destroy_window(WindowHandle window)
    {
        wz::platform::win32::w32_destroy_window(window);
    }

    bool window_should_close(WindowHandle handle)
    {
        return wz::platform::win32::w32_window_should_close(handle);
    }

    void pump_messages()
    {
        wz::platform::win32::w32_pump_messages();
    }

    bool poll_event(WindowHandle window, WindowEvent &out_event)
    {
        return wz::platform::win32::w32_poll_event(window, out_event);
    }

    // optional helper to drain all events with a callback, can be used for direct integration with engine event system
    void dispatch_event(WindowHandle window, const WindowEvent &e)
    {
        (void)window;
        (void)e;
    }

    void poll_events_wrapper(WindowHandle window)
    {
        wz::platform::win32::w32_pump_messages();

        platform::win32::drain_events(
            window,
            [](const WindowEvent &e, void *user)
            {
                auto window = *reinterpret_cast<WindowHandle *>(user);
                dispatch_event(window, e);
            },
            nullptr);
    }

}