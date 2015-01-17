/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_TASK_HPP_INCLUDED
#define STDEX_TASK_HPP_INCLUDED

#include <type_traits>

#   if defined(STDEX_HAS_STD_COROUTINE)
#   include <coroutine>
#   else
#   include <experimental/resumable>

namespace std
{
    template<class Promise = void>
    using coroutine_handle = experimental::resumable_handle<Promise>;

    using experimental::suspend_never;
    using experimental::suspend_always;
    using experimental::suspend_if;
}

#   define await __await
#   endif

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

        std::coroutine_handle<> _then;

        void notify()
        {
            if (_then)
                _then();
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
            _tag = tag::value;
            notify();
        }

        void set_exception(std::exception_ptr const& e)
        {
            new(&_e) std::exception_ptr(e);
            _tag = tag::exception;
            notify();
        }

    protected:

        promise_data() {}

        T extract_value()
        {
            return std::move(_val);
        }

        ~promise_data()
        {
            switch (_tag)
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
        tag _tag = tag::null;
        bool _token = false;
    };

    template<>
    struct promise_data<void> : promise_base
    {
        void set_result()
        {
            _tag = tag::value;
            notify();
        }

        void set_exception(std::exception_ptr const& e)
        {
            _e = e;
            _tag = tag::exception;
            notify();
        }

    protected:

        void extract_value() {}

        std::exception_ptr _e;
        tag _tag = tag::null;
        bool _token = false;
    };

    template<class F>
    auto await_result_test(F&& f) -> decltype(await_resume(f));

    template<class F>
    auto await_result_test(F&& f) -> decltype(f.await_resume());
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

            std::suspend_never initial_suspend()
            {
                return {};
            }

            auto final_suspend()
            {
                return std::suspend_if(transfer_ownership());
            }

            bool cancellation_requested() const
            {
                return false;
            }

        private:

            friend class task;

            T get()
            {
                if (this->_tag == task_detail::tag::value)
                    return this->extract_value();
                std::rethrow_exception(this->_e);
            }

            bool transfer_ownership()
            {
                return this->_token = !this->_token;
            }
        };

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

        bool await_ready() const noexcept
        {
            return _p->_tag != task_detail::tag::null;
        }

        void await_suspend(std::coroutine_handle<> cb) noexcept
        {
            _p->_then = cb;
        }

        T await_resume()
        {
            return _p->get();
        }

        ~task()
        {
            if (_p && !_p->transfer_ownership())
                std::coroutine_handle<promise_type>::from_promise(_p)();
        }

    private:

        promise_type* _p;
    };

    template<class F>
    using await_result_t = decltype(task_detail::await_result_test(std::declval<F>()));

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
