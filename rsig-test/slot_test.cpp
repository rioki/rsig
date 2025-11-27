// rsig - rioki's signal library
// Copyright (c) 2020-2025 Sean Farrell
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <functional>
#include <gtest/gtest.h>
#include <rsig/rsig.h>
#include <atomic>
#include <future>
#include <thread>
#include <chrono>

using namespace std::literals::chrono_literals;

TEST(slot, do_nothing)
{
    rsig::slot s;
}

TEST(slot, basic_raii_disconnect)
{
    rsig::signal<> void_signal;

    auto count = 0u;
    {
        rsig::slot s = void_signal.connect([&] () {
            count++;
        });

        EXPECT_EQ(0u, count);
        void_signal.emit();
        EXPECT_EQ(1u, count);
    }

    // slot is out of scope, should have disconnected
    void_signal.emit();
    EXPECT_EQ(1u, count);
}

TEST(slot, multiple_slots)
{
    rsig::signal<> void_signal;

    auto count1 = 0u;
    auto count2 = 0u;

    {
        rsig::slot s1 = void_signal.connect([&] () {
            count1++;
        });
        rsig::slot s2 = void_signal.connect([&] () {
            count2++;
        });

        EXPECT_EQ(0u, count1);
        EXPECT_EQ(0u, count2);
        void_signal.emit();
        EXPECT_EQ(1u, count1);
        EXPECT_EQ(1u, count2);
    }

    // both slots are out of scope
    void_signal.emit();
    EXPECT_EQ(1u, count1);
    EXPECT_EQ(1u, count2);
}

TEST(slot, move_semantics)
{
    rsig::signal<> void_signal;

    auto count = 0u;

    rsig::slot s1;
    {
        rsig::slot tmp  = void_signal.connect([&] () {
            count++;
        });
        s1 = std::move(tmp);
    }

    // moved-from tmp is dead, s1 should still be connected
    EXPECT_EQ(0u, count);
    void_signal.emit();
    EXPECT_EQ(1u, count);

    // explicit disconnect via API
    s1.disconnect();
    void_signal.emit();
    EXPECT_EQ(1u, count);
}

TEST(slot, signal_outlives_slot)
{
    rsig::signal<int> int_signal;

    auto value = 0;
    {
        rsig::slot s = int_signal.connect([&] (auto v) {
            value = v;
        });

        int_signal.emit(1);
        EXPECT_EQ(1, value);
    }

    // slot destroyed, no further updates
    int_signal.emit(2);
    EXPECT_EQ(1, value);
}

TEST(slot, slot_outlives_signal)
{
    auto value = 0;

    rsig::slot s;
    {
        rsig::signal<int> int_signal;

        s = int_signal.connect([&] (auto v) {
            value = v;
        });

        int_signal.emit(1);
        EXPECT_EQ(1, value);

        // int_signal is destroyed at the end of this scope
    }

    // here the underlying signal state is gone
    // destructor or manual disconnect of s must be safe
    s.disconnect(); // should be a no-op, not UB, not crash
    EXPECT_EQ(1, value);
}

class Mouse
{
public:
    rsig::connection on_move(std::function<void (int, int)> cb)
    {
        return move_signal.connect(std::move(cb));
    }

    void update()
    {
        auto x = std::rand();
        auto y = std::rand();

        move_signal.emit(x % 2 ? -1 : 1, y % 2 ? -1 : 1);
    }

private:
    rsig::signal<int, int> move_signal;
};

class AutoPlayerController
{
public:
    void activate(Mouse& mouse)
    {
        move_slot = mouse.on_move([this] (auto x, auto y) {
            control(x, y);
        });
    }

    void control(int x, int y)
    {
        u = x;
        v = y;
    }

    std::tuple<int, int> get_uv() const
    {
        return std::make_tuple(u, v);
    }

private:
    int u = 0;
    int v = 0;
    rsig::slot move_slot;
};

TEST(slot, life_time_raii)
{
    std::atomic<bool> running = true;
    Mouse mouse;

    auto f = std::async(std::launch::async, [&] () {
        while (running)
        {
            mouse.update();
        }
    });

    {
        AutoPlayerController ctrl;
        ctrl.activate(mouse);
        std::this_thread::sleep_for(10ms);
        auto [u, v] = ctrl.get_uv();
        EXPECT_NE(0, u);
        EXPECT_NE(0, v);
        // no explicit deactivate, relies on slot RAII
    }

    // AutoPlayerController is out of scope
    // if the slot does not deregister correctly this can crash
    std::this_thread::sleep_for(5ms);

    running = false;
    f.get();
}
