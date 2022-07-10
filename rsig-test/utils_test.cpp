//
// rsig - rioki's signal library
// Copyright (c) 2020 Sean Farrell
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

struct Foo
{
    void func() {}

    void const_func() const {}

    void noexcept_func() noexcept {}

    void const_noexcept_func() const noexcept {}
};

TEST(mem_fun, connect_all_types)
{
    Foo foo;
    auto f1 = rsig::mem_fun(&foo, &Foo::func);
    auto f2 = rsig::mem_fun(&foo, &Foo::const_func);
    auto f3 = rsig::mem_fun(&foo, &Foo::noexcept_func);
    auto f4 = rsig::mem_fun(&foo, &Foo::const_noexcept_func);
}

struct Counter
{
    unsigned int count = 0;

    void increment()
    {
        count++;
    }
};

TEST(mem_fun, function_is_invocated)
{
    Counter coutner;
    auto fun = rsig::mem_fun(&coutner, &Counter::increment);
    fun();
    EXPECT_EQ(1u, coutner.count);
}

