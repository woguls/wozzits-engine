#pragma once
#include <wozzits/event.h>

namespace wz::platform::win32::raw_input
{
    inline wz::event::EventSink *g_event_sink = nullptr;

    void w32_pump_messages();

    void collect(wz::event::EventSink &sink)
    {
        // Win32 already pushed events via WndProc
        // nothing required here for basic input

        // optional: synthesize derived events later
    }

}