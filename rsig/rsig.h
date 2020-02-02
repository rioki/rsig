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

#ifndef _RSIG_SIGNAL_H_
#define _RSIG_SIGNAL_H_

#include <cassert>
#include <functional>
#include <map>
#include <mutex>

namespace rsig
{
    struct connection
    {
        size_t id = 0;
        void* signal = nullptr;
    };

    template <typename... Args>
    class signal
    {
    public:
        signal() = default;
        ~signal() = default;

        connection observe(const std::function<void(Args...)>& fun);

        void remove_observer(connection id);

        size_t emit(Args... args) const;

    private:
        mutable
        std::mutex mutex;
        size_t last_id = 0;
        std::map<size_t, std::function<void (Args...)>> observers;

        signal(const signal<Args...>&) = delete;
        signal<Args...>& operator = (const signal<Args...>&) = delete;
    };

    template <typename... Args>
    connection signal<Args...>::observe(const std::function<void(Args...)>& fun)
    {
        std::scoped_lock<std::mutex> sl(mutex);
        if (!fun)
        {
            throw std::invalid_argument("Signal observer is invalid.");
        }

        auto id = ++last_id;
        observers[id] = fun;
        return {id, this};
    }

    template <typename... Args>
    void signal<Args...>::remove_observer(connection id)
    {
        if (id.signal != this)
        {
            throw std::invalid_argument("signal::remove_observer: mismatched connection");
        }

        std::scoped_lock<std::mutex> sl(mutex);
        auto i = observers.find(id.id);
        if (i == end(observers))
        {
            throw std::runtime_error("No observer with this id.");
        }
        observers.erase(i);
    }

    template <typename... Args>
    size_t signal<Args...>::emit(Args... args) const
    {
        std::scoped_lock<std::mutex> sl(mutex);
        for (auto& [id, fun] : observers)
        {
            assert(fun);
            fun(args...);
        }
        return observers.size();
    }
}

#endif
