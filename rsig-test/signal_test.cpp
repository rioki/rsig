//
// rsig - rioki's signal library
// Copyright (c) 2019 Sean Farrell
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
//

#include <functional>
#include <gtest/gtest.h>
#include <rsig/rsig.h>

TEST(signal, void_signal_observe)
{
    rsig::signal<> void_signal;

    auto count = 0u;
    void_signal.observe([&] () {
        count++;
    });

    EXPECT_EQ(0u, count);
    void_signal.emit();
    EXPECT_EQ(1u, count);
}

TEST(signal, int_signal_observe)
{
    rsig::signal<int> int_signal;

    auto count = 0u;
    auto value = 0;
    int_signal.observe([&](auto v) {
        count++;
        value = v;
    });

    EXPECT_EQ(0u, count);
    int_signal.emit(42);
    EXPECT_EQ(1u, count);
    EXPECT_EQ(42, value);
}

TEST(signal, string_int_signal_observe)
{
    rsig::signal<std::string, int> string_int_signal;

    auto count = 0u;
    auto value1 = std::string{};
    auto value2 = 0;
    string_int_signal.observe([&](auto v1, auto v2) {
        count++;
        value1 = v1;
        value2 = v2;
    });

    EXPECT_EQ(0u, count);
    string_int_signal.emit("Answer to the Ultimate Question of Life, the Universe, and Everything", 42);
    EXPECT_EQ(1u, count);
    EXPECT_EQ("Answer to the Ultimate Question of Life, the Universe, and Everything", value1);
    EXPECT_EQ(42, value2);
}

TEST(signal, unobserve)
{
    rsig::signal<> void_signal;

    auto count = 0u;
    auto c = void_signal.observe([&]() {
        count++;
    });

    void_signal.remove_observer(c);

    EXPECT_EQ(0u, count);
    void_signal.emit();
    EXPECT_EQ(0u, count);
}

TEST(signal, obverver_count)
{
    rsig::signal<> void_signal;

    auto c = void_signal.emit();
    EXPECT_EQ(0u, c);

    auto c1 = void_signal.observe([] () {});
    auto c2 = void_signal.observe([]() {});

    c = void_signal.emit();
    EXPECT_EQ(2u, c);

    void_signal.remove_observer(c2);

    c = void_signal.emit();
    EXPECT_EQ(1u, c);
}
