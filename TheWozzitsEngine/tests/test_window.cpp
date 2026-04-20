#include <iostream>
#include "../api/window.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    using namespace WZ::window;

    WindowDesc desc;
    desc.title = "Wozzits Window Test";
    desc.width = 800;
    desc.height = 600;

    WindowHandle window = create_window(desc);

    while (!window_should_close(window))
    {
        pump_messages();

        WindowEvent event{};
        while (poll_event(window, event))
        {
            // if (event.type == WindowEventType::Resize)
            // {
            //     std::cout << "Resize: "
            //               << event.resize.width << " x "
            //               << event.resize.height << std::endl;
            // }

            // if (event.type == WindowEventType::Close)
            // {
            //     std::cout << "Close event\n";
            // }
        }
    }
    destroy_window(window);

    return 0;
}