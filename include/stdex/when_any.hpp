/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_WHEN_ANY_HPP_INCLUDED
#define STDEX_WHEN_ANY_HPP_INCLUDED

#include <memory>
#include <stdex/task.hpp>

namespace stdex { namespace task_detail
{
    template<class T>
    struct callback_hook
    {
        coroutine_handle<> coro;
        T result;

        explicit operator bool() const
        {
            return coro;
        }

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
    detached_task hook(It it, std::shared_ptr<Callback> cp)
    {
        await *it;
        if (auto& callback = *cp)
        {
            callback(it);
        }
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
        auto cp = std::make_shared<callback_hook<It>>();
        for ( ; begin != end; ++begin)
        {
            if (begin->await_ready())
                return begin;
            hook(begin, cp);
        }
        return await *cp;
    }

    template<class... T>
    task<unsigned> when_any(task<T>&... t)
    {
        auto cp = std::make_shared<callback_hook<unsigned>>();
        unsigned i = 0;
        std::initializer_list<bool>
        {
            (hook(indexed_ptr<task<T>>{i++, &t}, cp), true)...
        };
        return await *cp;
    }
}

#endif