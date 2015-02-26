/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_COROUTINE_TASK_HPP_INCLUDED
#define STDEX_COROUTINE_TASK_HPP_INCLUDED

#include <atomic>
#include <functional>
#include <type_traits>
#include <stdex/coroutine/core.hpp>
#include <boost/assert.hpp>

namespace stdex { namespace task_detail
{
    template<class T>
    struct wrap_reference
    {
        using type = T;
    };

    template<class T>
    struct wrap_reference<T&>
    {
        using type = std::reference_wrapper<T>;
    };

    enum class tag : std::uint8_t
    {
        null
      , value
      , exception
    };

    struct promise_base
    {
    protected:

        std::atomic<coroutine_handle<>> _then {coroutine_handle<>()};

        void notify()
        {
            if (auto run = _then.exchange({}, std::memory_order_relaxed))
                run();
        }
    };

    template<class T>
    struct promise_data : promise_base
    {
        using val_t = typename wrap_reference<T>::type;

        template<class U>
        void set_result(U&& u)
        {
            new(&_val) val_t(std::forward<U>(u));
            _tag.store(tag::value, std::memory_order_release);
            notify();
        }

        void set_exception(std::exception_ptr const& e)
        {
            new(&_e) std::exception_ptr(e);
            _tag.store(tag::exception, std::memory_order_release);
            notify();
        }

    protected:

        promise_data() {}

        T&& extract_value()
        {
            return static_cast<T&&>(_val);
        }

        ~promise_data()
        {
            switch (_tag.load(std::memory_order_relaxed))
            {
            case tag::value:
                _val.~val_t();
                return;
            case tag::exception:
                _e.~exception_ptr();
            }
        }

        union
        {
            val_t _val;
            std::exception_ptr _e;
        };
        std::atomic<tag> _tag {tag::null};
        std::atomic<std::uint8_t> _token {false};
    };

    template<>
    struct promise_data<void> : promise_base
    {
        void set_result()
        {
            _tag.store(tag::value, std::memory_order_release);
            notify();
        }

        void set_exception(std::exception_ptr const& e)
        {
            _e = e;
            _tag.store(tag::exception, std::memory_order_release);
            notify();
        }

    protected:

        void extract_value() {}

        std::exception_ptr _e;
        std::atomic<tag> _tag {tag::null};
        std::atomic<std::uint8_t> _token {false};
    };

    struct manager
    {
        template<class Task>
        static void death_wakeup(Task& task)
        {
            return task.death_wakeup();
        }
    };
}}

namespace stdex
{
    template<class T>
    class task
    {
    public:

        struct promise_type : task_detail::promise_data<T>
        {
            task get_return_object()
            {
                return task(*this);
            }

            suspend_never initial_suspend()
            {
                return {};
            }

            auto final_suspend()
            {
                return suspend_if(transfer_ownership());
            }

            bool cancellation_requested() const
            {
                return false;
            }

        private:

            friend class task;

            std::add_rvalue_reference_t<T> get()
            {
                if (this->_tag.load(std::memory_order_acquire) == task_detail::tag::exception)
                    std::rethrow_exception(this->_e);
                return this->extract_value();
            }

            bool transfer_ownership()
            {
                return !this->_token.fetch_xor(true, std::memory_order_relaxed);
            }

            // We could put `_token` here, but for better member packing, it's
            // put inside the base promise_data<T>.
        };

        task() noexcept : _p() {}

        explicit task(promise_type& p) noexcept : _p(&p) {}

        task(task&& other) noexcept : _p(other._p)
        {
            other._p = nullptr;
        }

        task& operator=(task&& other) noexcept
        {
            ~task();
            return *new(this) task(std::move(other));
        }

        bool valid() const noexcept
        {
            return _p;
        }

        bool await_ready() const noexcept
        {
            return _p->_tag.load(std::memory_order_relaxed) != task_detail::tag::null;
        }

        bool await_suspend(coroutine_handle<> cb) noexcept
        {
            auto old = _p->_then.exchange(cb, std::memory_order_relaxed);
            BOOST_ASSERT_MSG(!old, "multiple coroutines await on same task");
            return _p->_tag.load(std::memory_order_relaxed) == task_detail::tag::null
                || !_p->_then.exchange({}, std::memory_order_relaxed);
        }

        T await_resume()
        {
            return _p->get();
        }

        ~task()
        {
            if (_p && !_p->transfer_ownership())
                coroutine_handle<promise_type>::from_promise(_p)();
        }

    private:

        friend struct task_detail::manager;

        // This should only be called if you can ensure that the contiuation
        // will finish right after wakeup without further suspend.
        void death_wakeup()
        {
            if (auto run = _p->_then.exchange({}, std::memory_order_relaxed))
                run();
        }

        promise_type* _p;
    };

#   if defined(STDEX_HAS_STD_COROUTINE)

    template<class F>
    inline task<await_result_t<F>> spawn(F&& f)
    {
        return await f;
    }

#   else

    template<class F, class R = await_result_t<F>>
    inline std::enable_if_t<std::is_void<R>::value, task<R>> spawn(F&& f)
    {
        await f;
    }

    template<class F, class R = await_result_t<F>>
    inline std::enable_if_t<!std::is_void<R>::value, task<R>> spawn(F&& f)
    {
        return await f;
    }

#   endif
}

#endif
