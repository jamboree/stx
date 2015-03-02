/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_COROUTINE_CORE_HPP_INCLUDED
#define STDEX_COROUTINE_CORE_HPP_INCLUDED

#include <atomic>
#include <stdex/atomic_count.hpp>

#   if defined(STDEX_HAS_STD_COROUTINE)

#   include <coroutine>

namespace stdex
{
    using std::coroutine_handle;
    using std::suspend_never;
    using std::suspend_always;
    using std::suspend_if;
}

#   else

#   include <experimental/resumable>

#       ifndef await
#       define await __await
#       endif

namespace stdex
{
    template<class Promise = void>
    using coroutine_handle = std::experimental::resumable_handle<Promise>;

    using std::experimental::suspend_never;
    using std::experimental::suspend_always;
    using std::experimental::suspend_if;
}

#   endif

namespace stdex { namespace coroutine_detail
{
    template<class F>
    auto await_result_test(F&& f) -> decltype(await_resume(f));

    template<class F>
    auto await_result_test(F&& f) -> decltype(f.await_resume());
}}

namespace stdex
{
    template<class F>
    using await_result_t =
        decltype(coroutine_detail::await_result_test(std::declval<F>()));

    template<class Promise>
    struct this_promise
    {
        this_promise() = default;

        bool await_ready() const noexcept
        {
            return false;
        }

        bool await_suspend(coroutine_handle<Promise> cb) noexcept
        {
            _p = &cb.promise();
            return false;
        }

        Promise& await_resume()
        {
            return *_p;
        }

    private:

        Promise* _p;
    };

    struct promise_base
    {
        suspend_never initial_suspend()
        {
            return {};
        }

        suspend_always final_suspend()
        {
            return {};
        }

        bool cancellation_requested() const
        {
            return !use_count.get();
        }

        stdex::atomic_count<unsigned> use_count {1};
    };

    template<class Promise = void>
    struct continuation;

    template<class Promise>
    struct continuation
    {
        using promise_type = Promise;

        continuation() : _handle() {}

        continuation(std::nullptr_t) : _handle() {}

        explicit continuation(Promise* promise)
            : _handle(coroutine_handle<Promise>::from_promise(promise))
        {}

        continuation(coroutine_handle<Promise> handle)
            : _handle(handle)
        {
            if (handle)
                count().inc();
        }

        continuation(continuation const& other)
            : continuation(other._handle)
        {}

        continuation(continuation&& other) noexcept
            : _handle(other._handle)
        {
            other._handle = nullptr;
        }

        ~continuation()
        {
            if (_handle && !count().dec())
                _handle();
        }

        continuation& operator=(continuation other) noexcept
        {
            swap(other);
            return *this;
        }

        void swap(continuation& other) noexcept
        {
            std::swap(_handle, other._handle);
        }

        void reset()
        {
            _handle = nullptr;
        }

        coroutine_handle<Promise> get_handle() const
        {
            return _handle;
        }

        void operator()() const noexcept
        {
            _handle();
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(_handle);
        }

        Promise& promise() noexcept
        {
            return _handle.promise();
        }

        Promise const& promise() const noexcept
        {
            return _handle.promise();
        }

    private:

        stdex::atomic_count<unsigned>& count()
        {
            return _handle.promise().promise_base::use_count;
        }

        coroutine_handle<Promise> _handle;
    };

    template<>
    struct continuation<void>
    {
        struct promise_type : promise_base
        {
            continuation get_return_object()
            {
                return continuation(this);
            }

            void set_result() {}

            void set_exception(std::exception_ptr const&)
            {
                std::terminate();
            }
        };

        continuation() : _handle() {}

        continuation(std::nullptr_t) : _handle() {}

        template<class Promise>
        explicit continuation(Promise* promise)
          : _handle(coroutine_handle<Promise>::from_promise(promise))
          , _count(&promise->promise_base::use_count)
        {}

        template<class Promise>
        continuation(coroutine_handle<Promise> handle)
            : _handle(handle)
        {
            if (handle)
            {
                _count = &handle.promise().promise_base::use_count;
                _count->inc();
            }
        }

        template<class Promise>
        continuation(continuation<Promise> const& coro)
            : continuation(coro._handle)
        {}

        continuation(continuation const& other)
            : _handle(other._handle), _count(other._count)
        {
            if (_handle)
                _count->inc();
        }

        continuation(continuation&& other) noexcept
            : _handle(other._handle), _count(other._count)
        {
            other._handle = nullptr;
        }

        ~continuation()
        {
            if (_handle && !_count->dec())
                _handle();
        }

        continuation& operator=(continuation other) noexcept
        {
            swap(other);
            return *this;
        }

        void swap(continuation& other) noexcept
        {
            std::swap(_handle, other._handle);
            std::swap(_count, other._count);
        }

        void reset()
        {
            _handle = nullptr;
        }

        coroutine_handle<> get_handle() const
        {
            return _handle;
        }

        void operator()() const noexcept
        {
            _handle();
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(_handle);
        }

    private:

        coroutine_handle<> _handle;
        stdex::atomic_count<unsigned>* _count;
    };

    template<class Derived>
    struct pausable
    {
        template<class Promise>
        auto await_suspend(coroutine_handle<Promise> cb) noexcept
        {
            return static_cast<Derived*>(this)->await_pause(continuation<Promise>(cb));
        }
    };

    struct detached_task
    {
        struct promise_type
        {
            detached_task get_return_object()
            {
                return{};
            }

            suspend_never initial_suspend()
            {
                return{};
            }

            suspend_never final_suspend()
            {
                return{};
            }

            bool cancellation_requested() const
            {
                return false;
            }

            void set_result() {}

            void set_exception(std::exception_ptr const& e) {}
        };

        detached_task() {}
    };

    struct attached_task
    {
        struct promise_type
        {
            attached_task get_return_object()
            {
                return attached_task(*this);
            }

            suspend_never initial_suspend()
            {
                return {};
            }

            auto final_suspend()
            {
                return suspend_if(transfer());
            }

            bool cancellation_requested() const
            {
                return !_active.load(std::memory_order_relaxed);
            }

            void set_result() {}

            void set_exception(std::exception_ptr const& e) {}

        private:

            friend struct attached_task;

            bool transfer()
            {
                return _active.fetch_xor(true, std::memory_order_relaxed);
            }

            std::atomic<std::uint8_t> _active {true};
        };

        attached_task() noexcept : _p() {}

        explicit attached_task(promise_type& p) noexcept : _p(&p) {}

        attached_task(attached_task&& other) noexcept
          : _p(other._p)
        {
            other._p = nullptr;
        }

        attached_task& operator=(attached_task&& other) noexcept
        {
            this->~attached_task();
            return *new(this) attached_task(std::move(other));
        }

        ~attached_task()
        {
            if (_p && !_p->transfer())
                coroutine_handle<promise_type>::from_promise(_p)();
        }

        void reset()
        {
            if (_p)
            {
                if (!_p->transfer())
                    coroutine_handle<promise_type>::from_promise(_p)();
                _p = nullptr;
            }
        }

    private:

        promise_type* _p;
    };

    struct coroutine : coroutine_handle<>
    {
        struct promise_type
        {
            coroutine get_return_object()
            {
                return coroutine_handle<promise_type>::from_promise(this);
            }

            suspend_never initial_suspend()
            {
                return {};
            }

            suspend_never final_suspend()
            {
                return {};
            }

            bool cancellation_requested() const
            {
                return cancelled;
            }

            void set_result() {}

            void set_exception(std::exception_ptr const&)
            {
                std::terminate();
            }

            bool cancelled = false;
        };

        coroutine(coroutine_handle<> coro)
          : coroutine_handle<>(coro)
        {}

        void cancel()
        {
            static_cast<coroutine_handle<promise_type>*>(static_cast<coroutine_handle<>*>(this))->promise().cancelled = true;
            (*this)();
        }
    };

    template<class F>
    inline coroutine make_coroutine(F f)
    {
        await suspend_always{};
        f();
    }

    template<class Task>
    inline auto until_awaken(Task& task)
    {
        struct awaiter
        {
            Task& task;

            bool await_ready() const noexcept
            {
                return false;
            }

            auto await_suspend(coroutine_handle<> cb) noexcept
            {
                return task.await_suspend(cb);
            }

            void await_resume() {}
        };
        return awaiter {task};
    }
}

#endif