#include <iostream>
#include <wozzits/window2.h>
#include <win32/platform_event.h>
#include <wozzits/input.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    using namespace wz::window;

    WindowDesc desc;
    desc.title = "Wozzits Window Test";
    desc.width = 800;
    desc.height = 600;

    WindowHandle window = create_window(desc);
    wz::input::init_raw_input();

    while (!window_should_close(window))
    {
        pump_messages();

        PlatformEvent event{};
        while (poll_event(window, event))
        {
            if (event.type == PlatformEvent::Type::Resize)
            {
                std::cout << "Resize: "
                          << event.resize.width << " x "
                          << event.resize.height << std::endl;
            }

            if (event.type == PlatformEvent::Type::Close)
            {
                std::cout << "Close event\n";
            }
        }
    }
    wz::input::shutdown_raw_input();
    destroy_window(window);

    return 0;
}