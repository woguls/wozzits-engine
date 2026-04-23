#pragma once

struct PlatformEvent
{
    enum class Type
    {
        Close,
        Resize,
        Key,
        MouseMove
    };

    Type type;

    union
    {
        struct
        {
            int key;
            bool pressed;
        } key;

        struct
        {
            int width;
            int height;
        } resize;

        struct
        {
            int x;
            int y;
        } mouse;
    };
};