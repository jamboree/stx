/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_SYNC_SPINLOCK_HPP_INCLUDED
#define STX_SYNC_SPINLOCK_HPP_INCLUDED

#include <atomic>

namespace stx
{
    class spinlock
    {
        std::atomic_flag _flag;

    public:
        spinlock() noexcept : _flag{ATOMIC_FLAG_INIT} {}
        spinlock(spinlock const&) = delete;
        spinlock& operator=(spinlock const&) = delete;

        void lock()
        {
            while (_flag.test_and_set(std::memory_order_acquire));
        }

        bool try_lock()
        {
            return !_flag.test_and_set(std::memory_order_acquire);
        }

        void unlock()
        {
            _flag.clear(std::memory_order_release);
        }
    };

    class shared_spinlock
    {
        enum ownership : uint32_t { unique = 1, shared = 2 };

    public:
        shared_spinlock() noexcept : _flags(0) {}

        shared_spinlock(shared_spinlock const&) = delete;
        shared_spinlock& operator=(shared_spinlock const&) = delete;

        void lock()
        {
            while (!try_lock());
        }

        void lock_shared()
        {
            while (!try_lock_shared());
        }

#if 0
        template<class Rep, class Period>
        bool try_lock_for(std::chrono::duration<Rep,Period> const& duration)
        {
            auto deadline = std::chrono::steady_clock::now() + duration;
            while (!try_lock())
            {
                if (std::chrono::steady_clock::now() >= deadline)
                    return false;
            }
            return true;
        }

        template<class Clock, class Duration>
        bool try_lock_until(std::chrono::time_point<Clock,Duration> const& deadline)
        {
            while (!try_lock())
            {
                if (Clock::now() >= deadline)
                    return false;
            }
            return true;
        }

        template<class Rep, class Period>
        bool try_lock_shared_for(std::chrono::duration<Rep,Period> const& duration)
        {
            auto deadline = std::chrono::steady_clock::now() + duration;
            while (!try_lock_shared())
            {
                if (std::chrono::steady_clock::now() >= deadline)
                    return false;
            }
            return true;
        }

        template<class Clock, class Duration>
        bool try_lock_shared_until(std::chrono::time_point<Clock,Duration> const& deadline)
        {
            while (!try_lock_shared())
            {
                if (Clock::now() >= deadline)
                    return false;
            }
            return true;
        }
#endif
        void unlock()
        {
            _flags.fetch_and(~unique, std::memory_order_release);
        }

        void unlock_shared()
        {
            _flags.fetch_sub(shared, std::memory_order_release);
        }


        bool try_lock()
        {
            std::uint32_t expect = 0;
            return _flags.compare_exchange_strong(expect, unique, std::memory_order_acq_rel);
        }

        bool try_lock_shared()
        {
            std::uint32_t value = _flags.fetch_add(shared, std::memory_order_acquire);
            if (value & unique)
            {
                _flags.fetch_sub(shared, std::memory_order_release);
                return false;
            }
            return true;
        }

    private:

        std::atomic<std::uint32_t> _flags;
    };
}

#endif