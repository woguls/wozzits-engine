#include <gtest/gtest.h>

#include <wozzits/hormuz_0.h>

using namespace wz::hormuz;
using wz::window::WindowEvent;
using wz::window::WindowEventType;

namespace
{
    struct TestState
    {
        int counter = 0;
        WindowEvent last_event{};
    };

    void on_event(const WindowEvent &e, void *user)
    {
        auto *state = static_cast<TestState *>(user);
        state->counter++;
        state->last_event = e;
    }
}

TEST(Hormuz, SingleListenerReceivesEvent)
{
    auto &bus = wz::hormuz::instance();
    bus.reset();
    TestState state;

    bus.subscribe(WindowEventType::Key, on_event, &state);

    WindowEvent e{};
    e.type = WindowEventType::Key;
    e.key.key = 42;
    e.key.state = wz::window::KeyState::Down;

    bus.publish(e);

    EXPECT_EQ(state.counter, 1);
    EXPECT_EQ(state.last_event.key.key, 42);
}

TEST(Hormuz, MultipleListenersReceiveEvent)
{
    auto &bus = wz::hormuz::instance();
    bus.reset();
    TestState a;
    TestState b;

    bus.subscribe(WindowEventType::Mouse, on_event, &a);
    bus.subscribe(WindowEventType::Mouse, on_event, &b);

    WindowEvent e{};
    e.type = WindowEventType::Mouse;
    e.mouse.x = 10;

    bus.publish(e);

    EXPECT_EQ(a.counter, 1);
    EXPECT_EQ(b.counter, 1);
}

TEST(Hormuz, EventTypeIsolation)
{
    auto &bus = wz::hormuz::instance();
    bus.reset();
    TestState key_state;
    TestState mouse_state;

    bus.subscribe(WindowEventType::Key, on_event, &key_state);
    bus.subscribe(WindowEventType::Mouse, on_event, &mouse_state);

    WindowEvent e{};
    e.type = WindowEventType::Key;
    e.key.key = 7;

    bus.publish(e);

    EXPECT_EQ(key_state.counter, 1);
    EXPECT_EQ(mouse_state.counter, 0);
}

TEST(Hormuz, PublishDoesNotCrashWithMaxListeners)
{
    auto &bus = wz::hormuz::instance();
    bus.reset();

    TestState state;

    for (int i = 0; i < 32; i++)
    {
        bus.subscribe(WindowEventType::Key, on_event, &state);
    }

    WindowEvent e{};
    e.type = WindowEventType::Key;

    bus.publish(e);

    EXPECT_EQ(state.counter, 32);
}