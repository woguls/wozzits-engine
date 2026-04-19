#include "../window.h"
#include "../../src/platform/win32.h"

namespace WZ::window
{
    WindowHandle create_window(const WindowDesc &desc)
    {
        OutputDebugStringA("create_window called\n");
        return WZ::platform::win32::create_window(desc);
    }

    void destroy_window(WindowHandle window)
    {
        HWND hwnd = (HWND)window.native;

        auto *data = WZ::platform::win32::GetWindowData(hwnd);

        if (data)
        {
            delete data;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        }

        DestroyWindow(hwnd);
    }

    bool window_should_close(WindowHandle handle)
    {
        HWND hwnd = (HWND)handle.native;
        auto *data = WZ::platform::win32::GetWindowData(hwnd);

        return data ? data->should_close : true;
    }

    bool poll_event(WindowHandle, WindowEvent &)
    {
        return false;
    }

    bool poll_key_event(WindowHandle, KeyEvent &)
    {
        return false;
    }

    bool poll_mouse_event(WindowHandle, MouseEvent &)
    {
        return false;
    }

    void pump_messages()
    {
        MSG msg;

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                // optionally set a global flag or propagate shutdown
                // or ignore if you're using window_should_close instead
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

}