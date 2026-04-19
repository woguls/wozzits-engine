#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <dwmapi.h>
#include <queue>

#include "../../api/window.h"

namespace WZ::platform::win32
{
    WZ::window::WindowHandle create_window(const WZ::window::WindowDesc &desc);
    void destroy_window(WZ::window::WindowHandle window);
    void pump_messages();

    struct Win32WindowData
    {
        inline static int s_next_id = 0;
        int id = 0;

        Win32WindowData()
            : id(++s_next_id)
        {
            char buf[128];
            sprintf_s(buf, "Win32WindowData constructed id=%d\n", id);
            OutputDebugStringA(buf);
        }

        ~Win32WindowData()
        {
            char buf[128];
            sprintf_s(buf, "Win32WindowData destroyed id=%d\n", id);
            OutputDebugStringA(buf);
        }

        HWND hwnd;
        bool should_close = false;

        std::queue<WZ::window::WindowEvent> event_queue;
    };

    static Win32WindowData *GetWindowData(HWND hwnd)
    {
        return reinterpret_cast<Win32WindowData *>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA));
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
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
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

        case WM_CLOSE:
        {
            auto *data = GetWindowData(hwnd);
            if (data)
                data->should_close = true;

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

            WZ::window::WindowEvent e;
            e.type = WZ::window::WindowEventType::Resize;
            e.resize.width = LOWORD(lParam);
            e.resize.height = HIWORD(lParam);

            data->event_queue.push(e);
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

            data->event_queue.push(e);
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

            data->event_queue.push(e);
            return 0;
        }
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    WZ::window::WindowHandle create_window(const WZ::window::WindowDesc &desc)
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = "WozzitsWindowClass";

        if (!RegisterClass(&wc))
        {
            OutputDebugStringA("RegisterClass FAILED\n");
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

        BOOL use_dark = TRUE;
        DwmSetWindowAttribute(
            hwnd,
            DWMWA_USE_IMMERSIVE_DARK_MODE,
            &use_dark,
            sizeof(use_dark));

        ShowWindow(hwnd, SW_SHOW);

        return WZ::window::WindowHandle{hwnd};
    }
}