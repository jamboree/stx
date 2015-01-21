/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_WHEN_ANY_HPP_INCLUDED
#define STDEX_WHEN_ANY_HPP_INCLUDED

#include <iterator>
#include <stdex/task.hpp>

namespace stdex { namespace task_detail
{
    template<class T>
    struct callback_hook
    {
        coroutine_handle<> coro;
        T result;

        void operator()(T val)
        {
            result = val;
            coro();
        }

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_suspend(coroutine_handle<> cb) noexcept
        {
            coro = cb;
        }

        T await_resume()
        {
            return result;
        }
    };

    template<class T>
    struct indexed_ptr
    {
        unsigned n;
        T* p;

        T& operator*() const
        {
            return *p;
        }

        operator unsigned() const
        {
            return n;
        }
    };

    template<class It, class Callback>
    inline harness hook(It it, Callback& cb)
    {
        await *it;
        cb(it);
    }

    template<class It>
    inline void unhook(It it, It end)
    {
        for ( ; it != end; ++it)
            harness::drop(*it);
    }

    template<class Callback, class T>
    inline unsigned recursive_hook(unsigned i, Callback& cb, task<T>& t)
    {
        if (t.await_ready())
            return i;
        hook(indexed_ptr<task<T>>{i, &t}, cb);
        return 0;
    }

    template<class Callback, class T, class... Ts>
    inline unsigned recursive_hook(unsigned i, Callback& cb, task<T>& t, task<Ts>&... ts)
    {
        if (t.await_ready())
            return i;
        if (unsigned n = recursive_hook(++i, cb, ts...))
            return n;
        hook(indexed_ptr<task<T>>{i, &t}, cb);
        return 0;
    }

    template<class T>
    inline void recursive_unhook(unsigned i, unsigned n, task<T>& t)
    {
        if (i != n)
            harness::drop(t);
    }

    template<class T, class... Ts>
    inline void recursive_unhook(unsigned i, unsigned n, task<T>& t, task<Ts>&... ts)
    {
        if (i != n)
            harness::drop(t);
        recursive_unhook(++i, n, ts...);
    }
}}

namespace stdex
{
    template<class Range>
    inline auto when_any(Range&& range)
    {
        using std::begin;
        using std::end;
        return when_any(begin(range), end(range));
    }

    template<class It>
    task<It> when_any(It begin, It end)
    {
        task_detail::callback_hook<It> cb;
        for (It it(begin); it != end; ++it)
        {
            if (it->await_ready())
            {
                task_detail::unhook(begin, it);
                return it;
            }
            task_detail::hook(it, cb);
        }
        It it(await cb), next(it);
        task_detail::unhook(begin, it);
        task_detail::unhook(++next, end);
        return it;
    }

    template<class... T>
    task<unsigned> when_any(task<T>&... t)
    {
        task_detail::callback_hook<unsigned> cb;
        if (unsigned n = task_detail::recursive_hook(1, cb, t...))
            return --n;
        unsigned n = await cb;
        task_detail::recursive_unhook(1, n, t...);
        return --n;
    }
}

#endif