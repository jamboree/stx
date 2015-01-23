/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_COROUTINE_ASYNC_STREAM_HPP_INCLUDED
#define STDEX_COROUTINE_ASYNC_STREAM_HPP_INCLUDED

#include <functional>
#include <type_traits>
#include <stdex/coroutine/core.hpp>
#include <boost/assert.hpp>
#include <boost/optional/optional.hpp>

namespace stdex { namespace async_stream_detail
{
    enum class tag : std::uint8_t
    {
        begin
      , null
      , value
      , exception
      , end
    };

    struct promise_base
    {
        suspend_always initial_suspend()
        {
            return {};
        }

        auto final_suspend()
        {
            if (_tag == tag::exception)
                _e.~exception_ptr();
            _tag = tag::end;
            return suspend_if(!_canceling && transfer_ownership());
        }

        bool cancellation_requested() const
        {
            return _canceling;
        }

        void set_result() {}

        void set_exception(std::exception_ptr const& e)
        {
            new(&_e) std::exception_ptr(e);
            _tag = tag::exception;
            if (_then)
                _then();
        }

    protected:

        promise_base() {}

        ~promise_base() {}

        bool transfer_ownership()
        {
            return _token = !_token;
        }

        coroutine_handle<> _then;
        union
        {
            void const* _ptr;
            std::exception_ptr _e;
        };
        tag _tag = tag::begin;
        bool _token = false;
        bool _canceling = false;
    };
}}

namespace stdex
{
    using boost::optional;
    using boost::none;

    template<class T>
    class async_stream
    {
    public:

        struct promise_type : async_stream_detail::promise_base
        {
            async_stream get_return_object()
            {
                return async_stream(*this);
            }

            auto yield_value(T const& val)
            {
                using async_stream_detail::tag;
                _ptr = std::addressof(val);
                _tag = tag::value;
                if (auto then = _then)
                {
                    _then = nullptr;
                    then();
                    return suspend_if(!_then && transfer_ownership());
                }
                return suspend_if(transfer_ownership());
            }

        private:

            friend class async_stream;

            optional<T> get()
            {
                using async_stream_detail::tag;
                switch (_tag)
                {
                case tag::value:
                    _tag = tag::null;
                    return *static_cast<T const*>(_ptr);
                case tag::exception:
                    _tag = tag::null;
                    std::rethrow_exception(_e);
                case tag::end:
                    return none;
                }
            }
        };

        explicit async_stream(promise_type& p) noexcept : _p(&p) {}

        async_stream(async_stream&& other) noexcept : _p(other._p)
        {
            other._p = nullptr;
        }

        async_stream& operator=(async_stream&& other) noexcept
        {
            ~async_stream();
            return *new(this) async_stream(std::move(other));
        }

        bool valid() const noexcept
        {
            return _p;
        }

        bool await_ready() const noexcept
        {
            using async_stream_detail::tag;
            if (_p->_tag == tag::begin)
                coroutine_handle<promise_type>::from_promise(_p)();
            return _p->_tag != tag::null;
        }

        void await_suspend(coroutine_handle<> cb) noexcept
        {
            BOOST_ASSERT_MSG(!_p->_then, "multiple coroutines await on same async_stream");
            _p->_then = cb;
        }

        optional<T> await_resume()
        {
            return _p->get();
        }

        ~async_stream()
        {
            if (_p)
            {
                _p->_canceling = true;
                if (!_p->transfer_ownership())
                    coroutine_handle<promise_type>::from_promise(_p)();
            }
        }

    private:

        promise_type* _p;
    };
}

#endif
