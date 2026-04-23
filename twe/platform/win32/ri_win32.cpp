#include <windows.h>
#include <wozzits/event.h>
#include <win32/ri_win32.h>
#include <malloc.h>

namespace
{
    HWND g_input_hwnd = nullptr;
}


namespace
{
    LRESULT CALLBACK RIWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_INPUT:
            wz::platform::win32::ri_process_input(lParam);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
}

namespace wz::platform::win32
{
    bool ri_init(HINSTANCE hInstance)
    {
        WNDCLASSW wc{};
        wc.lpfnWndProc = RIWndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"WozzitsRawInput";

        if (!RegisterClassW(&wc))
            return false;

        g_input_hwnd = CreateWindowExW(
            0,
            wc.lpszClassName,
            L"",
            0,
            0, 0, 0, 0,
            HWND_MESSAGE,
            nullptr,
            hInstance,
            nullptr
        );

        if (!g_input_hwnd)
            return false;

        RAWINPUTDEVICE rid{};

        // Mouse
        rid.usUsagePage = 0x01;
        rid.usUsage = 0x02;
        rid.dwFlags = RIDEV_INPUTSINK;
        rid.hwndTarget = g_input_hwnd;

        if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
            return false;

        return true;
    }
}

namespace wz::platform::win32
{
    void ri_shutdown()
    {
        if (g_input_hwnd)
        {
            DestroyWindow(g_input_hwnd);
            g_input_hwnd = nullptr;
        }
    }
}

namespace wz::platform::win32
{
    void ri_process_input(LPARAM lParam)
    {
        UINT size = 0;
        GetRawInputData((HRAWINPUT)lParam,
            RID_INPUT,
            nullptr,
            &size,
            sizeof(RAWINPUTHEADER));

        if (size == 0)
            return;

        BYTE* buffer = (BYTE*)alloca(size);

        if (GetRawInputData((HRAWINPUT)lParam,
            RID_INPUT,
            buffer,
            &size,
            sizeof(RAWINPUTHEADER)) != size)
            return;

        RAWINPUT* raw = (RAWINPUT*)buffer;

        wz::event::Event e{};
        e.source = wz::event::Event::Source::Platform;
        e.category = wz::event::Event::Category::Input;
        e.timestamp = wz::time::TimeSource::now_ticks();
        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            const RAWMOUSE& m = raw->data.mouse;

            e.type = wz::event::Event::Type::MouseMove;

            // relative movement (raw input is perfect for this)
            // you can refine later (accumulation happens in input system)
        }
        else if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            const RAWKEYBOARD& k = raw->data.keyboard;

            e.type = (k.Flags & RI_KEY_BREAK)
                ? wz::event::Event::Type::KeyPressUp
                : wz::event::Event::Type::KeyPressDown;

            // store key in a temporary way (you may extend Event later)
        }

        wz::event::event_queue.try_push(e);
    }
}

