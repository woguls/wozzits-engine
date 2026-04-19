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
    WZ::window::WindowHandle w32_create_window(const WZ::window::WindowDesc &desc);
    void w32_destroy_window(WZ::window::WindowHandle window);
    void w32_destroy_window(WZ::window::WindowHandle window);
    bool w32_window_should_close(WZ::window::WindowHandle window);
    bool w32_poll_event(WZ::window::WindowHandle window, WZ::window::WindowEvent &out_event);
    void w32_pump_messages();

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

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

}