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
}

#   endif

namespace stdex
{
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
                return attached_task(_canceled);
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
                return _canceled.load(std::memory_order_relaxed);
            }

            void set_result() {}

            void set_exception(std::exception_ptr const& e) {}

        private:

            std::atomic<bool> _canceled {false};
        };

        explicit attached_task(std::atomic<bool>& canceled)
          : _canceled(&canceled)
        {}

        attached_task(attached_task&& other) noexcept
          : _canceled(other._canceled)
        {
            other._canceled = nullptr;
        }

        attached_task& operator=(attached_task&& other) noexcept
        {
            this->~attached_task();
            return *new(this) attached_task(std::move(other));
        }

        ~attached_task()
        {
            _canceled->store(true, std::memory_order_relaxed);
        }

    private:

        std::atomic<bool>* _canceled;
    };
}

namespace stdex { namespace coroutine_detail
{
    template<class F>
    auto await_result_test(F&& f) -> decltype(await_resume(f));

    template<class F>
    auto await_result_test(F&& f) -> decltype(f.await_resume());

    struct yield_to
    {
        coroutine_handle<>& coro;

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_suspend(coroutine_handle<> cb) noexcept
        {
            coro = cb;
        }

        void await_resume() {}
    };

    template<class F>
    inline detached_task create_coroutine(coroutine_handle<>& coro, F&& f)
    {
        await yield_to{coro};
        f();
    }
}}

namespace stdex
{
    template<class F>
    using await_result_t =
        decltype(coroutine_detail::await_result_test(std::declval<F>()));

    template<class F>
    inline coroutine_handle<> make_coroutine_handle(F&& f)
    {
        coroutine_handle<> ret;
        coroutine_detail::create_coroutine(ret, std::forward<F>(f));
        return ret;
    }
}

#endif
