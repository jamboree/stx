/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2015-2017 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef STX_TRAITS_FIND_HPP_INCLUDED
#define STX_TRAITS_FIND_HPP_INCLUDED

#include <algorithm>
#include <stx/utility/priority.hpp>

namespace stx { namespace traits_detail
{
    using std::begin;
    using std::end;

    template<class Container, class T>
    inline auto find_impl(priority<1>, Container&& con, T const& val) -> decltype(con.find(val))
    {
        return con.find(val);
    }

    template<class Container, class T>
    inline auto find_impl(priority<0>, Container&& con, T const& val)
    {
        return std::find(begin(std::forward<Container>(con)), end(std::forward<Container>(con)), val);
    }

    template<class Container, class T>
    inline auto find(Container&& con, T const& val)
    {
        return find_impl(priority<1>{}, std::forward<Container>(con), val);
    }

    struct find_fn
    {
        template<class Container, class T>
        auto operator()(Container&& con, T const& val) const ->
            decltype(find(std::forward<Container>(con), val))
        {
            return find(std::forward<Container>(con), val);
        }
    };
}}

namespace stx { namespace traits
{
    constexpr traits_detail::find_fn find{};
}}

#endif