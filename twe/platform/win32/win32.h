#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <dwmapi.h>
#include <queue>

#include <wozzits/window.h>
#include <wozzits/spsc_queue.h>

namespace wz::platform::win32
{
    using WindowEventQueue = wz::core::internal::SPSCQueue<wz::window::WindowEvent>;
    static bool registered = false;

    wz::window::WindowHandle w32_create_window(const wz::window::WindowDesc &desc);
    void w32_destroy_window(wz::window::WindowHandle window);
    bool w32_window_should_close(wz::window::WindowHandle window);
    bool w32_poll_event(wz::window::WindowHandle window, wz::window::WindowEvent &out_event);
    void w32_pump_messages();
    bool drain_events(wz::window::WindowHandle window,
                      void (*callback)(const wz::window::WindowEvent &, void *),
                      void *user);

    struct Win32WindowData
    {
        static constexpr size_t WINDOW_EVENT_QUEUE_SIZE = 1024; // arbitrary power-of-two capacity for event queue

        inline static int s_next_id = 0;
        int id = 0;

        Win32WindowData()
            : id(++s_next_id),
              event_queue(WINDOW_EVENT_QUEUE_SIZE)
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

        HWND hwnd = nullptr;
        bool should_close = false;

        // IMPORTANT: this is now the ONLY bridge to engine thread
        WindowEventQueue event_queue;
    };

    static Win32WindowData *GetWindowData(HWND hwnd)
    {
        return reinterpret_cast<Win32WindowData *>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

}