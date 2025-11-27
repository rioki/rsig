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

#pragma once

#include <cassert>
#include <functional>
#include <map>
#include <mutex>
#include <memory>

namespace rsig
{
    using connection_id = size_t;

    struct basic_signal_state
    {
        virtual ~basic_signal_state() = default;
        virtual void disconnect(connection_id id) = 0;
    };

    template <typename... Args>
    struct signal_state : basic_signal_state
    {
        mutable std::mutex                              mutex;
        connection_id                                   last_id = 0;
        std::map<size_t, std::function<void (Args...)>> observers;

        connection_id connect(std::function<void(Args...)> fun)
        {
            if (!fun)
            {
                throw std::invalid_argument("Signal observer is invalid.");
            }

            auto lk = std::lock_guard(mutex);
            auto id = ++last_id;
            observers.emplace(id, std::move(fun));
            return id;
        }

        void disconnect(connection_id id) override
        {
            std::lock_guard lk(mutex);
            auto c = observers.erase(id);
            if (c != 1)
            {
                throw std::runtime_error("Invalid connection id.");
            }
        }

        size_t emit(Args... args)
        {
            auto lk = std::lock_guard(mutex);
            for (auto& [id, fun] : observers)
            {
                fun(args...);
            }
            return observers.size();
        }
    };

    //! Handle to a signal / observer connection.
    //!
    //! @note The connection struct is to be considered opaque to the user.
    struct connection
    {
        connection_id                     id = 0u;
        std::weak_ptr<basic_signal_state> wstate;

        void disconnect()
        {
            if (auto state = wstate.lock())
            {
                state->disconnect(id);
                wstate = {};
                id     = 0u;
            }
        }
    };

    //! A thread safe signal multiplexer.
    //!
    //! @note The signal class is thread safe. You can connect, disconnect and
    //! emit from multiple threads, just keep the object alive. The thread that
    //! emits the signal is the same that will call the functions. The signal
    //! is secured by a mutex, thus it may create contention when emitting
    //! a signal with a function that runs long.
    template <typename... Args>
    class signal
    {
    public:
        //! Constructor
        signal()
        : state(std::make_shared<signal_state<Args...>>()) {}

        //! Move Constructor
        signal(signal<Args...>&& other) noexcept
        : state(std::move(other.state)) {}

        //! Destructor
        ~signal() = default;

        //! Move Assignment
        signal<Args...>& operator = (signal<Args...>&& other) noexcept
        {
            if (this != &other)
            {
                state = std::move(other.state);
            }
            return *this;
        }

        //! Connect an observer to the signal.
        //!
        //! @param fun the lambda function that will be called when emit is called.
        //! @return the connection for this observer
        //!
        //! @warning If the context, like lambda captures, lifetime is shorter
        //! than the signal, the observer must be disconnected.
        connection connect(std::function<void(Args...)> fun)
        {
            assert(state);
            return connection{
                .id     = state->connect(std::move(fun)),
                .wstate = state
            };
        }

        //! Disconnect an observer.
        //!
        //! @param con the connection returned by connect
        void disconnect(connection con)
        {
            assert(state);
            state->disconnect(con.id);
        }

        //! Emit a signal.
        //!
        //! Calls all observer functions with the given arguments and returns
        //! the number of called functions.
        //!
        //! @param args the values of this signal event
        //! @return the number of called functions
        size_t emit(Args... args) const
        {
            assert(state);
            return state->emit(std::forward<Args>(args)...);
        }

    private:
        std::shared_ptr<signal_state<Args...>> state;

        signal(const signal<Args...>&) = delete;
        signal<Args...>& operator = (const signal<Args...>&) = delete;
    };


    //! Helper for Automatic Disconnection
    //!
    //! The slot class takes a connection and will safely disconenct from
    //! the signal, if the signal still exists. The signal and slot may have
    //! varying lifetimes and this will handle it gracefully.
    class slot
    {
    public:
        //! Create empty slot.
        slot() = default;

        //! Assign a connection to a slot.
        slot(rsig::connection conn)
        : conn(std::move(conn)) {}

        //! Move a slot.
        slot(slot&& other) noexcept
        : conn(std::exchange(other.conn, {})) {}

        //! Automatically disconnect from signal, if present and valid.
        ~slot()
        {
            disconnect();
        }

        //! Disconnect and move a slot.
        slot& operator=(slot&& other) noexcept
        {
            if (this != &other)
            {
                disconnect();
                conn = std::exchange(other.conn, {});
            }
            return *this;
        }

        //! Dissconnect from signal, if present and valid.
        //!
        //! @note Callign disconnect is allways safe. The worst that
        //! can happen is that is does nothing.
        void disconnect()
        {
            conn.disconnect();
        }

    private:
        rsig::connection conn;
    };

    template<typename Class, class Ret, class... Args>
    using method_pointer = Ret(Class::*)(Args...);

    template<typename Class, class Ret, class... Args>
    using method_pointer_const = Ret(Class::*)(Args...) const;

    template<typename Class, class Ret, class... Args>
    using method_pointer_ne = Ret(Class::*)(Args...) noexcept;

    template<typename Class, class Ret, class... Args>
    using method_pointer_const_ne = Ret(Class::*)(Args...) const noexcept;

    template <typename Class, typename Ret, typename... Args> [[deprecated]]
    std::function<Ret (Args...)> mem_fun(Class* that, method_pointer<Class, Ret, Args...> method)
    {
        return [that, method] (Args... args) {
            (that->*method)(args...);
        };
    }

    template <typename Class, typename Ret, typename... Args> [[deprecated]]
    std::function<Ret (Args...)> mem_fun(Class* that, method_pointer_const<Class, Ret, Args...> method)
    {
        return [that, method] (Args... args) {
            (that->*method)(args...);
        };
    }

    template <typename Class, typename Ret, typename... Args> [[deprecated]]
    std::function<Ret (Args...)> mem_fun(Class* that, method_pointer_ne<Class, Ret, Args...> method)
    {
        return [that, method] (Args... args) {
            (that->*method)(args...);
        };
    }

    template <typename Class, typename Ret, typename... Args> [[deprecated]]
    std::function<Ret (Args...)> mem_fun(Class* that, method_pointer_const_ne<Class, Ret, Args...> method)
    {
        return [that, method] (Args... args) {
            (that->*method)(args...);
        };
    }
}
