#include "win32.h"

namespace
{
    using WindowEventQueue = WZ::core::SPSCQueue<WZ::window::WindowEvent>;

    inline void push_window_event(WindowEventQueue &q,
                                  WZ::window::WindowEvent e)
    {
        q.push(std::move(e)); // drop-on-full policy
    }
}
namespace WZ::platform::win32
{

    bool drain_events(WZ::window::WindowHandle window,
                      void (*callback)(const WZ::window::WindowEvent &, void *),
                      void *user)
    {
        auto *data = GetWindowData((HWND)window.native);
        if (!data)
            return false;

        WZ::window::WindowEvent e;

        while (data->event_queue.try_pop(e))
        {
            callback(e, user);
        }

        return true;
    }

    void w32_destroy_window(WZ::window::WindowHandle window)
    {
        HWND hwnd = (HWND)window.native;

        DestroyWindow(hwnd);
    }

    bool w32_window_should_close(WZ::window::WindowHandle handle)
    {
        HWND hwnd = (HWND)handle.native;
        auto *data = WZ::platform::win32::GetWindowData(hwnd);

        return data ? data->should_close : true;
    }

    bool w32_poll_event(WZ::window::WindowHandle window, WZ::window::WindowEvent &out_event)
    {
        if (!window.valid())
            return false;
        auto *data = GetWindowData((HWND)window.native);
        if (!data)
            return false;

        return data->event_queue.try_pop(out_event);
    }

    void w32_pump_messages()
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

    WZ::window::WindowHandle w32_create_window(const WZ::window::WindowDesc &desc)
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = "WozzitsWindowClass";

        if (!registered)
        {
            if (!RegisterClass(&wc))
            {
                OutputDebugStringA("RegisterClass FAILED\n");
            }

            registered = true;
        }

        auto *data = new Win32WindowData();

        HWND hwnd = CreateWindowEx(
            0,
            wc.lpszClassName,
            desc.title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            desc.width, desc.height,
            nullptr,
            nullptr,
            hInstance,
            data);

        if (!hwnd)
        {
            OutputDebugStringA("CreateWindowEx FAILED\n");
            delete data;
            return {};
        }
        data->hwnd = hwnd;

        BOOL use_dark = TRUE;
        DwmSetWindowAttribute(
            hwnd,
            DWMWA_USE_IMMERSIVE_DARK_MODE,
            &use_dark,
            sizeof(use_dark));

        ShowWindow(hwnd, SW_SHOW);

        return WZ::window::WindowHandle{hwnd};
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_CLOSE)
            OutputDebugStringA("WM_CLOSE\n");

        if (msg == WM_DESTROY)
            OutputDebugStringA("WM_DESTROY\n");

        if (msg == WM_SIZE)
            OutputDebugStringA("WM_SIZE\n");

        if (msg == WM_NCCREATE)
            OutputDebugStringA("WM_NCCREATE\n");

        switch (msg)
        {
        case WM_NCCREATE:
        {
            auto *cs = (CREATESTRUCT *)lParam;
            auto *data = (Win32WindowData *)cs->lpCreateParams;

            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
            return TRUE;
        }

        case WM_ERASEBKGND:
            return 1; // tell Windows "we handled it"

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);

            HBRUSH brush = CreateSolidBrush(RGB(20, 20, 20));
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_NCDESTROY:
        {
            auto *data = GetWindowData(hwnd);

            if (data)
            {
                delete data;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            }
            return 0;
        }

        case WM_CLOSE:
        {
            auto *data = GetWindowData(hwnd);
            if (data)
            {
                data->should_close = true;
                WZ::window::WindowEvent e{};
                e.type = WZ::window::WindowEventType::Close;

                push_window_event(data->event_queue, e);
            }
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE:
        {
            auto *data = GetWindowData(hwnd);
            if (!data)
                return DefWindowProc(hwnd, msg, wParam, lParam);

            WZ::window::WindowEvent e{};
            e.type = WZ::window::WindowEventType::Resize;
            e.resize.width = LOWORD(lParam);
            e.resize.height = HIWORD(lParam);

            // DROP ON FULL (intentional policy)
            push_window_event(data->event_queue, e);

            return 0;
        }

        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            auto *data = GetWindowData(hwnd);
            if (!data)
                return DefWindowProc(hwnd, msg, wParam, lParam);

            WZ::window::WindowEvent e;
            e.type = WZ::window::WindowEventType::Key; // you don't have this yet (see note below)
            e.key.key = (int)wParam;
            e.key.state = (msg == WM_KEYDOWN)
                              ? WZ::window::KeyState::Down
                              : WZ::window::KeyState::Up;

            push_window_event(data->event_queue, e);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            auto *data = GetWindowData(hwnd);
            if (!data)
                return DefWindowProc(hwnd, msg, wParam, lParam);

            WZ::window::WindowEvent e;
            e.type = WZ::window::WindowEventType::Mouse;
            e.mouse.x = GET_X_LPARAM(lParam);
            e.mouse.y = GET_Y_LPARAM(lParam);

            push_window_event(data->event_queue, e);
            return 0;
        }
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}