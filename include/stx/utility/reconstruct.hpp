/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_UTILITY_RECONSTRUCT_HPP_INCLUDED
#define STX_UTILITY_RECONSTRUCT_HPP_INCLUDED

#include <type_traits>
#include <initializer_list>
#include <stx/utility/priority.hpp>

namespace stx
{
    template<class T, class... As>
    using is_nothrow_reconstructible = std::integral_constant<bool,
        std::is_nothrow_constructible<T, As...>::value &&
        std::is_nothrow_destructible<T>::value>;
}

namespace stx { namespace detail
{
    template<class T, class... As>
    inline std::enable_if_t<is_nothrow_reconstructible<T, As...>::value>
    reconstruct_impl(priority<2>, T& t, As&&... as) noexcept
    {
        t.~T();
        new(&t) T(std::forward<As>(as)...);
    }

    template<class T, class... As>
    inline std::enable_if_t<
      std::is_nothrow_move_constructible<T>::value &&
      std::is_nothrow_destructible<T>::value>
    reconstruct_impl(priority<1>, T& t, As&&... as)
    {
        T tmp(std::forward<As>(as)...);
        t.~T();
        new(&t) T(std::move(tmp));
    }

    template<class T, class... As>
    inline void reconstruct_impl(priority<0>, T& t, As&&... as)
    {
        t = T(std::forward<As>(as)...);
    }
}}

namespace stx
{
    template<class T, class... As>
    inline std::enable_if_t<std::is_constructible<T, As...>::value>
    reconstruct(T& t, As&&... as) noexcept(
        is_nothrow_reconstructible<T, As...>::value)
    {
        detail::reconstruct_impl(priority<2>{}, t, std::forward<As>(as)...);
    }

    template<class T, class U>
    inline std::enable_if_t<std::is_constructible<T, std::initializer_list<U>>::value>
    reconstruct(T& t, std::initializer_list<U> ilist) noexcept(
        is_nothrow_reconstructible<T, std::initializer_list<U>>::value)
    {
        detail::reconstruct_impl(priority<2>{}, t, ilist);
    }
}

#endif