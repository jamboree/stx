/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_RYPPL_SIGNAL_HPP_INCLUDED
#define STDEX_RYPPL_SIGNAL_HPP_INCLUDED

#include <vector>
#include <memory>
#include <atomic>
#include <boost/optional/optional.hpp>
#include <stdex/spinlock.hpp>
#include <stdex/coroutine/core.hpp>

namespace stdex { namespace ryppl
{
    enum class tag : std::uint8_t
    {
        null
      , value
      , exception
    };

    template<class T>
    struct signal
    {
        struct promise_type
        {
            promise_type()
			{
				_followers_mutex.clear();
			}

            ~promise_type()
            {
				switch (_tag)
				{
				case tag::value:
					_val.~T();
					return;
				case tag::exception:
					_e.~exception_ptr();
				}
            }

            signal get_return_object()
            {
                return signal(*this);
            }

            suspend_never initial_suspend()
            {
                return {};
            }

            auto final_suspend()
            {
                return suspend_if(--_use_count);
            }

            bool cancellation_requested() const
            {
                return false;
            }

            void set_result() {}

            void set_exception(std::exception_ptr const& e)
            {
                new(&_e) std::exception_ptr(e);
                _tag = tag::exception;
                notify_all();
            }

			template<class U>
            auto yield_value(U&& u)
            {
				if (_tag == tag::null)
				{
					new(&_val) val_t(std::forward<U>(u));
					_tag = tag::value;
				}
				else
					_val = std::forward<U>(u);
                return suspend_if(!notify_all());
            }

        private:

            friend class signal;

            T const& get()
            {
                if (_tag == tag::value)
                    return _val;
                std::rethrow_exception(_e);
            }

			boost::optional<T> get_optional()
			{
				switch (_tag)
				{
				case tag::null: return boost::none;
				case tag::value: return _val;
				case tag::exception: std::rethrow_exception(_e);
				}
			}

			void add_follower(coroutine_handle<> cb)
			{
				spinlock lock(_followers_mutex);
				_followers.push_back(cb);
			}

            bool notify_all()
            {
				std::vector<coroutine_handle<>> followers;
				{
					spinlock lock(_followers_mutex);
					followers.swap(_followers);
				}
                for (auto& f : followers)
                    f();
                return !_followers.empty();
            }

            std::vector<coroutine_handle<>> _followers;
            union
            {
                T _val;
                std::exception_ptr _e;
            };
			std::atomic<unsigned> _use_count = 1;
            tag _tag = tag::null;
			std::atomic_flag _followers_mutex/* = ATOMIC_FLAG_INIT*/; // MSVC bug
        };

        explicit signal(promise_type& p) noexcept : _p(&p)
        {
            ++_p->_use_count;
        }

        signal(signal const& other) noexcept : _p(other._p)
        {
            ++_p->_use_count;
        }

        signal(signal&& other) noexcept : _p(other._p)
        {
            other._p = nullptr;
        }

        signal& operator=(signal other) noexcept
        {
            swap(other);
            return *this;
        }

        void swap(signal& other) noexcept
        {
            std::swap(_p, other._p);
        }

        ~signal()
        {
            if (_p && !--_p->_use_count)
                coroutine_handle<promise_type>::from_promise(_p)();
        }

        bool valid() const noexcept
        {
            return _p;
        }

		boost::optional<T> current() const
        {
            return _p->get_optional();
        }

        bool await_ready() const noexcept
        {
            return _p->_tag != tag::null;
        }

        void await_suspend(coroutine_handle<> cb) noexcept
        {
            _p->add_follower(cb);
        }

        T await_resume()
        {
            return _p->get();
        }

    private:
        
        promise_type* _p;
    };
#if 0
    template<class F, class T>
    auto make_process(F f, T val)
    {
        return [f, val]
        {
            if (keep_alive)
            {
                f(val);
                if (!notify_followers())
                {
                    keep_alive = false;
                    curr = nullptr;
                }
            }
        };
    }

    template<class F, class T>
    signal<T> lift(boost::asio::io_service& io, signal<T> const& sig, F&& f)
    {
        boost::asio::io_service::strand strand(io);
        //boost::lockfree::queue<std::shared_ptr<T>> queue;
        std::atomic<bool> keep_alive(true);

        for ( ; ; )
        {
            if (auto curr = sig.curr())
            {
                strand.post(make_process(f, curr));
            }
            for ( ; ; )
            {
                auto curr = await sig;
                if (!keep_alive)
                    break;
                strand.post(make_process(f, curr));
            }
        }
        ////


    }
#endif
}}

#endif