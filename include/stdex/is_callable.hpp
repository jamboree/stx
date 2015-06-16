/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STDEX_IS_CALLABLE_HPP_INCLUDED
#define STDEX_IS_CALLABLE_HPP_INCLUDED

#include <type_traits>

namespace stdex
{
    struct dont_care;
}

namespace stdex { namespace detail
{
    template<class T, class U>
    struct is_resultable : std::is_convertible<T, U> {};

    template<class T>
    struct is_resultable<T, dont_care> : std::true_type {};

    template<class F, class R, class... Ts>
    auto is_callable(int, Ts&&... ts) -> std::enable_if_t<is_resultable<
        decltype(std::declval<F>()(std::forward<Ts>(ts)...)), R>::value
      , std::true_type>;

    template<class F, class R>
    auto is_callable(float, ...) -> std::false_type;
}}

namespace stdex
{
    template<class Sig, class R = dont_care>
    struct is_callable;
    
    template<class F, class... Ts, class R>
    struct is_callable<F(Ts...), R>
      : decltype(detail::is_callable<F, R>(0, std::declval<Ts>()...))
    {};
}

#endif
