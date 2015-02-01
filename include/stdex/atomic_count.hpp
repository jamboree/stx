/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_ATOMIC_COUNT_HPP_INCLUDED
#define STDEX_ATOMIC_COUNT_HPP_INCLUDED

#include <atomic>

namespace stdex
{
    template<class T>
    struct atomic_count
    {
        atomic_count(T n) noexcept : _n(n) {}

        void inc()
        {
            _n.fetch_add(1, std::memory_order_relaxed);
        }

        bool dec()
        {
            return _n.fetch_sub(1, std::memory_order_relaxed) != 1;
        }

        T get() const
        {
            return _n.load(std::memory_order_relaxed);
        }

    private:

        std::atomic<unsigned> _n;
    };
}

#endif
