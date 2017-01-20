/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_TYPE_TRAITS_IS_CALLABLE_HPP_INCLUDED
#define STX_TYPE_TRAITS_IS_CALLABLE_HPP_INCLUDED

#include <type_traits>

namespace stx
{
    struct dont_care;
}

namespace stx { namespace detail
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

namespace stx
{
    template<class Sig, class R = dont_care>
    struct is_callable;
    
    template<class F, class... Ts, class R>
    struct is_callable<F(Ts...), R>
      : decltype(detail::is_callable<F, R>(0, std::declval<Ts>()...))
    {};
}

#endif