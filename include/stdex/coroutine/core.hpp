/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_COROUTINE_CORE_HPP_INCLUDED
#define STDEX_COROUTINE_CORE_HPP_INCLUDED

#include <atomic>

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

            void set_exception(std::exception_ptr const& e)
            {
                std::rethrow_exception(e);
            }

            bool cancelled = false;
        };

        coroutine(coroutine_handle<> coro)
          : coroutine_handle<>(coro)
        {}

        void cancel()
        {
            static_cast<coroutine_handle<promise_type>*>(this)->cancelled = true;
            (*this)();
        }
    };

    template<class F>
    inline coroutine make_coroutine(F f)
    {
        await suspend_always{};
        f();
    }
}

#endif