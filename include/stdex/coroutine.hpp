/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_COROUTINE_HPP_INCLUDED
#define STDEX_COROUTINE_HPP_INCLUDED

#   if defined(STDEX_HAS_STD_COROUTINE)
#   include <coroutine>
#   else
#   include <experimental/resumable>

namespace stdex
{
    template<class Promise = void>
    using coroutine_handle = std::experimental::resumable_handle<Promise>;

    using std::experimental::suspend_never;
    using std::experimental::suspend_always;
    using std::experimental::suspend_if;
}

#   define await __await
#   endif

#endif